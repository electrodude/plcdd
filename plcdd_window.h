#ifndef PLCDD_WINDOW_H
#define PLCDD_WINDOW_H

#include <stddef.h>

struct plcdd_display; // from plcdd_display.h

struct plcdd_window
{
	struct plcdd_display *display;

	unsigned int y;
	unsigned int x;

	unsigned int width;

	unsigned int dispoff;

	size_t len;
	char *buf;
};

int plcdd_window_new(struct plcdd_window *window, struct plcdd_display *display, unsigned int y, unsigned int x, unsigned int width, size_t len);
void plcdd_window_dtor(struct plcdd_window *window);

void plcdd_window_draw(struct plcdd_window *window);

#endif
