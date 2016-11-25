#ifndef PLCDD_CUSTOMCHAR
#define PLCDD_CUSTOMCHAR

struct plcdd_display; // from plcdd_display.h

int plcdd_customchar_update(struct plcdd_display *display);
int plcdd_customchar_define(struct plcdd_display *display, int i, const char def[8]);
int plcdd_customchar_alloc2(struct plcdd_display *display);
int plcdd_customchar_alloc(struct plcdd_display *display, const char def[8]);
int plcdd_customchar_free(struct plcdd_display *display, int i);

void plcdd_customchar_from_asciiart(char def[8], const char *p);

#endif
