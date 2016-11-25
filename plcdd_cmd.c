#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "plcdd_cmd.h"

#define DEBUG 0

ssize_t plcdd_write(int fd, const char *buf, size_t count)
{
	return write(fd, buf, count);
}

ssize_t plcdd_write_char(int fd, char c)
{
	return plcdd_write(fd, &c, 1);
}

size_t plcdd_mvstr(int fd, unsigned char pos, unsigned int len, const char *str)
{
	size_t msglen = 1 + len;
#if DEBUG
	char buf[msglen + 1];
#else
	char buf[msglen];
#endif

	char *p = buf;

	*p++ = pos;

	for (size_t i = 0; i < len; i++)
	{
		unsigned char c = str[i];

		if (c == '\0') break;
		if (c == '\x0B') c = '\0';

		if (!((c < 8) || (32 <= c && c < 127)))
		{
			fprintf(stderr, "warn : filtered bad char %02x, %d of %d, string at (%d, %d)\n", c, i, len, PLCDD_POS_TO_Y_X(pos));
			c = '?';
		}

		*p++ = c;
	}

#if DEBUG
	*p = '\0';

	fprintf(stderr, "debug: at (%d, %d) %d chars: '%s'\n", PLCDD_POS_TO_Y_X(pos), len, &buf[1]);
#endif

	ssize_t n_bytes = p - buf;

	ssize_t n_out = plcdd_write(fd, buf, n_bytes);

#if DEBUG
	if (n_out != n_bytes)
	{
		fprintf(stderr, "debug: only wrote %zd out of %zd bytes\n", n_out, n_bytes);
	}
#endif

	return n_out > 0 ? n_out - 1 : 0;
}

int plcdd_define_customchar(int fd, unsigned int i, const char def[8])
{
	char buf[9];

	buf[0] = 0xF8 + (i & 7);
	buf[1] = def[0];
	buf[2] = def[1];
	buf[3] = def[2];
	buf[4] = def[3];
	buf[5] = def[4];
	buf[6] = def[5];
	buf[7] = def[6];
	buf[8] = def[7];

	return plcdd_write(fd, buf, 9) != 9;
}
