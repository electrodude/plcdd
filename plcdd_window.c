#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "plcdd_display.h"

#include "plcdd_window.h"

struct plcdd_window *plcdd_window_new_at(struct plcdd_window *window, struct plcdd_display *display, unsigned int y, unsigned int x, unsigned int width, size_t len)
{
	if (width > len)
	{
		fprintf(stderr, "warn : plcdd_window_new_at(%p, %p, %u, %u, %u, %zu): width > len\n", window, display, y, x, width, len);
	}

	char *buf = malloc(len + 1);

	if (buf == NULL)
	{
		window->len = 0;
		return NULL;
	}

	memset(buf, ' ', len + 1);
	//buf[len] = '\0';

	window->display = display;
	window->dispoff = 0;
	window->y = y;
	window->x = x;
	window->width = width;
	window->len = len;
	window->buf = buf;

	return window;
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

struct plcdd_window *plcdd_window_new(struct plcdd_display *display, unsigned int y, unsigned int x, unsigned int width, size_t len)
{
	struct plcdd_window *window = malloc(sizeof(*window));

	return plcdd_window_new_at(window, display, y, x, width, len);
}

void plcdd_window_free(struct plcdd_window *window)
{
	plcdd_window_dtor(window);

	free(window);
}

void plcdd_window_draw(struct plcdd_window *window)
{
	plcdd_display_draw(window->display, window->y, window->x, window->width, &window->buf[window->dispoff]);
}
