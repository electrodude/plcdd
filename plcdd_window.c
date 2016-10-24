#include <stdlib.h>
#include <string.h>

#include "plcdd_display.h"

#include "plcdd_window.h"

int plcdd_window_new(struct plcdd_window *window, struct plcdd_display *display, unsigned int y, unsigned int x, size_t len)
{
	char *buf = malloc(len + 1);

	if (buf == NULL)
	{
		window->len = 0;
		return 1;
	}

	memset(buf, ' ', len + 1);
	//buf[len] = '\0';

	window->display = display;
	window->dispoff = 0;
	window->displen = len;
	window->y = y;
	window->x = x;
	window->len = len;
	window->buf = buf;

	return 0;
}

void plcdd_window_dtor(struct plcdd_window *window)
{
	if (window == NULL) return;

	window->display = NULL;

	window->len = 0;

	if (window->buf != NULL)
	{
		free(window->buf);
	}

	window->buf = NULL;
}

void plcdd_window_draw(struct plcdd_window *window)
{
	plcdd_display_draw(window->display, window->y, window->x, window->displen, &window->buf[window->dispoff]);
}
