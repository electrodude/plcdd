#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#include "plcdd_cmd.h"

#include "plcdd_display.h"

int plcdd_display_open(struct plcdd_display *display, const char *device, int baud, size_t rows, size_t cols)
{
	display->fd = open(device, O_WRONLY);

	if (display->fd == -1)
	{
		return 1;
	}

	// TODO: set baud

	display->rows = rows;
	display->cols = cols;

	size_t n = display->rows*display->cols;

	display->curr = malloc(sizeof(char[n]));
	memset(display->curr, PLCDD_UNKNOWN, n);

	display->next = malloc(sizeof(char[n]));
	memset(display->next, PLCDD_UNKNOWN, n);

	display->backlight_curr = PLCDD_UNKNOWN;
	display->backlight_next = PLCDD_UNKNOWN;

	display->status_curr = PLCDD_UNKNOWN;
	display->status_next = PLCDD_UNKNOWN;

	return 0;
}

void plcdd_display_close(struct plcdd_display *display)
{
	if (display == NULL) return;

	close(display->fd);
	display->fd = -1;

	free(display->curr);
	display->curr = NULL;

	free(display->next);
	display->next = NULL;
}

void plcdd_display_clear(struct plcdd_display *display)
{
	size_t n = display->rows*display->cols;

	memset(display->next, ' ', n);
}

void plcdd_display_draw(struct plcdd_display *display, unsigned int y, unsigned int x, unsigned int len, const char *str)
{
	if (y >= display->rows)
	{
		printf("warn : on nonexistant line %d\n", y);
		return;
	}

	if (x >= display->cols)
	{
		printf("warn : on nonexistant col  %d\n", x);
		return;
	}

	if (x + len > display->cols)
	{
		len = display->cols - x;
		printf("warn : truncated to %d chars\n", len);
	}

	memcpy(&display->next[y*display->cols + x], str, len);
}

static inline int plcdd_display_set_property(struct plcdd_display *display, char *curr, char next)
{
	if (next == PLCDD_UNKNOWN)
	{
		*curr = PLCDD_UNKNOWN;
		return 3;
	}
	else if (next != *curr)
	{
		if (plcdd_write_char(display->fd, next) == 1)
		{
			*curr = next;
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		return 2;
	}
}

int plcdd_display_update_backlight(struct plcdd_display *display)
{
	return plcdd_display_set_property(display, &display->backlight_curr, display->backlight_next);
}

int plcdd_display_update_status(struct plcdd_display *display)
{
	return plcdd_display_set_property(display, &display->status_curr, display->status_next);
}

void plcdd_display_update(struct plcdd_display *display)
{
	plcdd_display_set_property(display, &display->status_curr, PLCDD_STATUS_ON);

	int dirty;

	do
	{
		dirty = 0;

		size_t n = display->rows*display->cols;

#if 0
		if (rand() < RAND_MAX / 4)
		{
			// mark a random stretch of characters as dirty
			size_t j = rand() % n;
			for (size_t i = 0; i < 8; i++)
			{
				if (display->next[(j + i) % n] != PLCDD_UNKNOWN)
				{
					display->curr[(j + i) % n] = PLCDD_UNKNOWN;
				}
			}
		}
#endif
		for (size_t i = 0; i < n; i++)
		{
			if (display->next[i] == '\0')
			{
				display->curr[i] = '\0';
			}
		}

		for (size_t i = 0; i < n; i++)
		{
			//printf("debug: i = %d\n", i);
			if (display->next[i] != display->curr[i])
			{
				size_t j = i + 1;
				while ((display->next[j] != display->curr[j] || (j+1 < n && display->next[j+1] != display->curr[j+1] && display->next[j+1] != '\0')) && j <= n)
				{
					//printf("debug: j = %d\n", j);
					j++;
				}

				//printf("debug: update [%d, %d)\n", i, j);

				unsigned int len = j - i;

				size_t len_out = plcdd_mvstr(display->fd, plcdd_pos_adjust(i, display->cols), len, &display->next[i]);

				if (len_out != len) dirty = 1;

				memcpy(&display->curr[i], &display->next[i], len_out);

				i = j;
			}
		}

	} while (dirty);

	plcdd_display_update_backlight(display);
	plcdd_display_update_status(display);
}

void plcdd_display_customchar_define(struct plcdd_display *display, unsigned int i, char def[8])
{
	plcdd_customchar_define(display->fd, i, def);
}

void plcdd_customchar_from_asciiart(char *def, const char *p)
{
	char *def_end = &def[8];
	for (; *p && def != def_end; p++)
	{
		if (*p == '\n')
		{
			def++;
			*def = 0;
		}
		else if (isspace(*p))
		{
			*def = (*def << 1) | 0;
		}
		else
		{
			*def = (*def << 1) | 1;
		}
	}

	while (def != def_end)
	{
		def++;
		*def = 0;
	}
}
