#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "plcdd_display.h"
#include "plcdd_window.h"

/*
  01234567890123456789  
 +--------------------+ 
1|                    |1
2|                    |2
3|                    |3
4|                    |4
 +--------------------+ 
  01234567890123456789  


  01234567890123456789  
 +--------------------+ 
1|                    |1
2|                    |2
3|                    |3
4|x.xx,x.xx   HH:MM:SS|4
4|tTT˚ nn/nnn HH:MM:SS|4
4|Wkd   mm/dd HH:MM:SS|4
 +--------------------+
  01234567890123456789
*/

int backlight_timer = 0;

void backlight_timeout(int seconds)
{
	if (backlight_timer < seconds)
	{
		backlight_timer = seconds;
	}
}

int main(int argc, char **argv)
{
	struct plcdd_display display;
	plcdd_display_open(&display, "/dev/ttyAMA0", 9600, 4, 20);
	display.status_next = PLCDD_STATUS_ON;

	struct plcdd_window window_time;
	plcdd_window_new(&window_time, &display, 3, 12,  8);

	struct plcdd_window window_load;
	plcdd_window_new(&window_load, &display, 3,  0, 14);

	struct plcdd_window window_processes;
	plcdd_window_new(&window_processes, &display, 3,  5,  16);
	window_processes.displen = 6;

	struct plcdd_window window_temp;
	plcdd_window_new(&window_temp, &display, 3,  0,  4);

	struct plcdd_window window_date;
	plcdd_window_new(&window_date, &display, 3,  0,  11);

	struct plcdd_window window_users;
	plcdd_window_new(&window_users, &display, 3,  0,  11);

	backlight_timeout(60);

	float t1, t5, t15;
	int runnable, total;
	int lastpid;

	int temp1000;

	tzset();

	FILE *loadavg = fopen("/proc/loadavg", "r");
	if (loadavg == NULL)
	{
		fprintf(stderr, "warn : failed to open /proc/loadavg");
	}

	FILE *cputemp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	if (cputemp == NULL)
	{
		fprintf(stderr, "warn : failed to open /sys/class/thermal/thermal_zone0/temp");
	}

	while (1)
	{

#if 1
		memset(&display.next[3*display.cols], ' ', display.cols);
#else
		plcdd_display_clear(&display);
#endif

		if (backlight_timer > 0)
		{
			display.backlight_next = PLCDD_BACKLIGHT_ON;
			backlight_timer--;
		}
		else
		{
			display.backlight_next = PLCDD_BACKLIGHT_OFF;
		}

		time_t t = time(NULL);
		struct tm tm;
		localtime_r(&t, &tm);
		size_t len = strftime(window_time.buf, window_time.len + 1, "%H:%M:%S", &tm);
		plcdd_window_draw(&window_time);


		if (loadavg != NULL)
		{
			rewind(loadavg);
			fflush(loadavg);
			int n = fscanf(loadavg, "%f %f %f %d/%d %d", &t1, &t5, &t15, &runnable, &total, &lastpid);
			if (n != 6)
			{
				fprintf(stderr, "warn : reading /proc/loadavg: only found %d out of 6 fields\n", n);
			}

			if (t1 > 1.0f || t5 > 1.0f)
			{
				backlight_timeout(30);
			}
		}

		switch ((t / 4) % 4)
		{
			case 0:
			{
#if 1
				strftime(window_date.buf, window_date.len + 1, "%a %m/%d  ", &tm);
#else
				strftime(window_date.buf, window_date.len + 1, "%a %d %b", tm);
#endif
				plcdd_window_draw(&window_date);
				break;
			}
			case 1:
			{
				if (cputemp != NULL)
				{
					rewind(cputemp);
					fflush(cputemp);
					int n = fscanf(cputemp, "%d", &temp1000);
					if (n != 1)
					{
						fprintf(stderr, "warn : reading cpu temp: only found %d out of 1 fields\n", n);
					}
				}
				snprintf(window_temp.buf, window_temp.len + 1, "%d\x07             ", (temp1000 + 500) / 1000);
				plcdd_window_draw(&window_temp);

				snprintf(window_processes.buf, window_processes.len + 1, "%d/%d            ", runnable, total);
				plcdd_window_draw(&window_processes);
				break;
			}
			case 2:
			{
				snprintf(window_load.buf, window_load.len + 1, "%.2f,%.2f,%.2f   ", t1, t5, t15);
				window_load.dispoff = t % 4;
				plcdd_window_draw(&window_load);
				break;
			}
			case 3:
			{
				snprintf(window_users.buf, window_users.len + 1, "TODO users    ");
				plcdd_window_draw(&window_users);
				break;
			}
		}

		plcdd_display_flush(&display);

		struct timespec one_sec = {.tv_sec = 1, .tv_nsec = 0,};
		nanosleep(&one_sec, &one_sec);
	}

	fclose(cputemp);
	fclose(loadavg);

	plcdd_display_close(&display);
}
