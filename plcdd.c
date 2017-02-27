#define _POSIX_C_SOURCE 199309L
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "plcdd_display.h"
#include "plcdd_customchar.h"
#include "plcdd_window.h"
#include "plcdd_progress.h"

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

time_t backlight_end = 0;

void backlight_timeout(int seconds)
{
	time_t now = time(NULL);

	if (now + seconds > backlight_end)
	{
		backlight_end = now + seconds;
	}
}

void parse_meminfo(char *p, const char *pe, double *memtotal, double *memactive)
{
	while (p != pe)
	{
		const char *key = p;
		while (p != pe && *p != ':') p++;
		const char *keye = p;
		*p++ = '\0';

		while (p != pe && isspace(*p)) p++;

		double value = 0;
		while (p != pe && isdigit(*p))
		{
			value = value*10 + (*p - '0');
			p++;
		}

		while (p != pe && isspace(*p)) p++;

		const char *unit = p;
		while (p != pe && *p != '\n') p++;
		const char *unite = p;
		*p++ = '\0';

		while (p != pe && !isalnum(*p)) p++;

		switch (*unit)
		{
			case 'k': value *= 1000.0                    ; unit++; break;
			case 'K': value *= 1024.0                    ; unit++; break;
			case 'm': value *= 1000.0*1000               ; unit++; break;
			case 'M': value *= 1024.0*1024               ; unit++; break;
			case 'g': value *= 1000.0*1000*1000          ; unit++; break;
			case 'G': value *= 1024.0*1024*1024          ; unit++; break;
			case 't': value *= 1000.0*1000*1000*1000     ; unit++; break;
			case 'T': value *= 1024.0*1024*1024*1024     ; unit++; break;
			case 'p': value *= 1000.0*1000*1000*1000*1000; unit++; break;
			case 'P': value *= 1024.0*1024*1024*1024*1024; unit++; break;
		}

		if (!strcmp(key, "MemTotal"))
		{
			*memtotal = value;
			//fprintf(stderr, "%s: %g %s\n", key, value, unit);
		}
		if (!strcmp(key, "Active"))
		{
			*memactive = value;
			//fprintf(stderr, "%s: %g %s\n", key, value, unit);
		}
	}
	//fprintf(stderr, "\n");
}

static const char *degree_ascii =
" ##  \n"
"#  # \n"
"#  # \n"
" ##  \n"
"     \n"
"     \n"
"     \n"
"     \n";

int main(int argc, char **argv)
{
	const char *path = "/dev/ttyAMA0";
	int baud = 9600;

	if (argc >= 2)
	{
		path = argv[1];
	}

	if (argc >= 3)
	{
		baud = strtol(argv[2], NULL, 10);
	}

	tzset();

	struct plcdd_display display;
	plcdd_display_open(&display, path, baud, 4, 20);
	display.status_next = PLCDD_STATUS_ON;

	char degree_def[8];
	plcdd_customchar_from_asciiart(degree_def, degree_ascii);
	plcdd_customchar_define(&display, 7, degree_def);

	plcdd_progress_init(&display);

	struct plcdd_window window_time;
	plcdd_window_new_at(&window_time, &display, 3, 12, 8, 8);

	struct plcdd_window window_load;
	plcdd_window_new_at(&window_load, &display, 3,  0, 11, 14);

	struct plcdd_window window_processes;
	plcdd_window_new_at(&window_processes, &display, 3, 5, 6, 16);

	struct plcdd_window window_temp;
	plcdd_window_new_at(&window_temp, &display, 3,  0, 4, 11);

	struct plcdd_window window_date;
	plcdd_window_new_at(&window_date, &display, 3,  0, 11, 11);

	struct plcdd_window window_mem;
	plcdd_window_new_at(&window_mem, &display, 3,  0, 11, 11);

	struct plcdd_window window_users;
	plcdd_window_new_at(&window_users, &display, 3,  0, 11, 11);

	backlight_timeout(60);

	float t1, t5, t15;
	int runnable, total;
	int lastpid;

	int temp1000;

	FILE *loadavg = fopen("/proc/loadavg", "r");
	if (loadavg == NULL)
	{
		fprintf(stderr, "warn : failed to open /proc/loadavg");
	}

	int meminfo = open("/proc/meminfo", O_RDONLY);
	if (meminfo < 0)
	{
		fprintf(stderr, "warn : failed to open /proc/meminfo");
	}

	FILE *cputemp = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
	if (cputemp == NULL)
	{
		fprintf(stderr, "warn : failed to open /sys/class/thermal/thermal_zone0/temp");
	}

	while (1)
	{
		struct timespec now;
		clock_gettime(CLOCK_REALTIME, &now);

		time_t t = now.tv_sec;

		struct tm tm;
		localtime_r(&t, &tm);

		int tm_sec = tm.tm_sec;

		int mode = (t / 4) % 4;

#if 1
		plcdd_display_clear_line(&display, 3); // only clear last line
#else
		plcdd_display_clear(&display);
#endif

		if (t < backlight_end)
		{
			display.backlight_next = PLCDD_BACKLIGHT_ON;
		}
		else
		{
			display.backlight_next = PLCDD_BACKLIGHT_OFF;
		}

		size_t len = strftime(window_time.buf, window_time.len + 1, "%H:%M:%S", &tm);
		plcdd_window_draw(&window_time);


		if (mode == 1 || mode == 2)
		{
			if (loadavg != NULL)
			{
				//fprintf(stderr, "read loadavg\n");
				rewind(loadavg);
				fflush(loadavg);
				int n = fscanf(loadavg, "%f %f %f %d/%d %d", &t1, &t5, &t15, &runnable, &total, &lastpid);
				if (n != 6)
				{
					fprintf(stderr, "warn : reading loadavg: only found %d out of 6 fields\n", n);
				}

				if (t1 > 1.0f || t5 > 0.75f || t15 > 0.5f)
				{
					backlight_timeout(60);
				}
			}
		}

		switch (mode)
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
					//fprintf(stderr, "read cputemp\n");
					rewind(cputemp);
					fflush(cputemp);
					int n = fscanf(cputemp, "%d", &temp1000);
					if (n != 1)
					{
						fprintf(stderr, "warn : reading cputemp: only found %d out of 1 fields\n", n);
					}
				}

				if (temp1000 > 50000)
				{
					backlight_timeout(30);
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
				double memtotal = 0.0;
				double memactive = 0.0;
				if (meminfo >= 0)
				{
					//fprintf(stderr, "read meminfo\n");
					lseek(meminfo, 0, SEEK_SET);

#define BUFLEN 8192
					char buf[BUFLEN];
					ssize_t n = read(meminfo, &buf, BUFLEN);

					char *p  =  buf;
					char *pe = &buf[n];

					parse_meminfo(p, pe, &memtotal, &memactive);
#undef BUFLEN
				}
				snprintf(window_mem.buf, window_mem.len + 1, "%.1f/%.0fMB         ", memactive / (1024.0*1024), memtotal / (1024.0*1024));
				plcdd_window_draw(&window_mem);
				break;
			}
			case 4:
			{
				snprintf(window_users.buf, window_users.len + 1, "TODO users    ");
				plcdd_window_draw(&window_users);
				break;
			}
		}

		plcdd_display_update(&display);

		struct timespec one_sec = {.tv_sec = 1, .tv_nsec = 0,};
		nanosleep(&one_sec, &one_sec);
	}

	fclose(cputemp);
	fclose(loadavg);

	plcdd_display_close(&display);

	return 0;
}
