#ifndef PLCDD_PROGRESS_H
#define PLCDD_PROGRESS_H

#define PLCDD_PROGRESS_CHARS_N 6

struct plcdd_window; // from plcdd_window.h
struct plcdd_display; // from plcdd_display.h

struct plcdd_progress
{
	struct plcdd_window window;

	int curr;
};

void plcdd_progress_init(struct plcdd_display *display);

int plcdd_progress_new(struct plcdd_progress *progress, struct plcdd_display *display, unsigned int y, unsigned int x, unsigned int width, size_t len);
int plcdd_progress_dtor(struct plcdd_progress *progress);

int plcdd_progress_max(struct plcdd_progress *progress);

void plcdd_progress_draw(struct plcdd_progress *progress);

int plcdd_progress_set(struct plcdd_progress *progress, int value);

#endif
