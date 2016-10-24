#ifndef PLCDD_DISPLAY_H
#define PLCDD_DISPLAY_H

#include <stddef.h>

#define PLCDD_UNKNOWN               0x00

#define PLCDD_BACKLIGHT_ON          0x11
#define PLCDD_BACKLIGHT_OFF         0x12

#define PLCDD_STATUS_OFF            0x15
#define PLCDD_STATUS_ON             0x16
#define PLCDD_STATUS_ON_CHAR_BLINK  0x17
#define PLCDD_STATUS_ON_CURSOR_ON   0x18
#define PLCDD_STATUS_ON_CURSOR_CHAR 0x19

struct plcdd_display
{
	char *curr; // current display contents
	char *next; // pending display contents

	size_t rows;
	size_t cols;

	char backlight_curr;
	char backlight_next;
	char status_curr;
	char status_next;

	int fd;
};

int plcdd_display_open(struct plcdd_display *display, const char *device, int baud, size_t rows, size_t cols);
void plcdd_display_close(struct plcdd_display *display);

void plcdd_display_clear(struct plcdd_display *display);

void plcdd_display_draw(struct plcdd_display *display, unsigned int y, unsigned int x, unsigned int len, const char *str);

int plcdd_display_update_backlight(struct plcdd_display *display);
int plcdd_display_update_status(struct plcdd_display *display);

void plcdd_display_update(struct plcdd_display *display);

void plcdd_display_customchar_define(struct plcdd_display *display, unsigned int i, char def[8]);
void plcdd_customchar_from_asciiart(char *def, const char *p);

#endif
