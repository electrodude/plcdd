#include <string.h>

#include "plcdd_display.h"
#include "plcdd_window.h"

#include "plcdd_progress.h"

static char plcdd_progress_chars[6];

void plcdd_progress_init(struct plcdd_display *display)
{
	plcdd_progress_chars[0] = ' ';

	char bar_def[8] = {0};
	for (int i = 1; i <= 5; i++)
	{
		for (int y = 0; y < 8; y++)
		{
			bar_def[y] |= 1 << (5 - i);
		}

		plcdd_progress_chars[i] = plcdd_display_customchar_define_alloc(display, bar_def);
	}
}

int plcdd_progress_new(struct plcdd_progress *progress, struct plcdd_display *display, unsigned int line)
{
	plcdd_window_new(&progress->window, display, line, 0, 20, 20);

	progress->curr = 0;

	memset(progress->window.buf, plcdd_progress_chars[0], progress->window.len);
}

int plcdd_progress_dtor(struct plcdd_progress *progress)
{
	plcdd_window_dtor(&progress->window);
}

void plcdd_progress_draw(struct plcdd_progress *progress)
{
	plcdd_window_draw(&progress->window);
}

int plcdd_progress_set(struct plcdd_progress *progress, int new)
{
	int old = progress->curr;

	if (new / 6 < old / 6)
	{
		for (int x = old / 6; x > new / 6; x--)
		{
			progress->window.buf[x] = plcdd_progress_chars[0];
		}
	}
	else if (new / 6 > old / 6)
	{
		for (int x = old / 6; x < new / 6; x++)
		{
			progress->window.buf[x] = plcdd_progress_chars[5];
		}
	}

	if (new != old)
	{
		progress->window.buf[new/6] = plcdd_progress_chars[new % 6];
	}

	progress->curr = new;
}

