#ifndef PLCDD_CMD_H
#define PLCDD_CMD_H

// Low level stuff; these are not the functions you are looking for

#include <sys/types.h>

#define PLCDD_MAXCOLS 20

#define PLCDD_POS_TO_Y_X(_pos) (((_pos) - 0x80) / PLCDD_MAXCOLS), (((_pos) - 0x80) % PLCDD_MAXCOLS)

#define plcdd_pos_to_y(_pos) (((_pos) - 0x80) / PLCDD_MAXCOLS)
#define plcdd_pos_to_x(_pos) (((_pos) - 0x80) % PLCDD_MAXCOLS)

static inline unsigned char plcdd_yx_to_pos(unsigned int y, unsigned int x)
{
	return 0x80 + y*PLCDD_MAXCOLS + x;
}

static inline unsigned char plcdd_pos_adjust(unsigned char pos, unsigned int cols)
{
	unsigned int x = pos % cols;
	unsigned int y = pos / cols;

	return 0x80 + y*PLCDD_MAXCOLS + x;
}

static inline unsigned char plcdd_customchar(int fd, unsigned int i)
{
	unsigned char c = i & 7;

	if (c == 0) c = 8;

	return c;
}

ssize_t plcdd_write(int fd, const char *buf, size_t count);
ssize_t plcdd_write_char(int fd, char c);

size_t plcdd_mvstr(int fd, unsigned char pos, unsigned int len, const char *str);

int plcdd_define_customchar(int fd, unsigned int i, const char def[8]);

#endif
