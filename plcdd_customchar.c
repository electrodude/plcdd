#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>

#include "plcdd_cmd.h"
#include "plcdd_display.h"

#include "plcdd_customchar.h"

int plcdd_customchar_update(struct plcdd_display *display)
{
	for (size_t i = 0; i < 8; i++)
	{
		if (display->customchar_mask_next & (1 << i))
		{
			if (memcmp(display->customchar_defs_curr[i], display->customchar_defs_next[i], 8))
			{
				int rc = plcdd_define_customchar(display->fd, i, display->customchar_defs_next[i]);
				if (!rc)
				{
					memcpy(display->customchar_defs_curr[i], display->customchar_defs_next[i], 8);

					display->customchar_mask_curr |=  (1 << i);
				}
				else
				{
					fprintf(stderr, "failed to update customchar %zu\n", i);
				}
			}
		}
		else
		{
			display->customchar_mask_curr &= ~(1 << i);
		}
	}
}

int plcdd_customchar_define(struct plcdd_display *display, int i, const char def[8])
{
	if (i == '\x0B') i = 0;
	if (i < 0 || i >= 8) return 1;

	memcpy(display->customchar_defs_next[i], def, 8);

	display->customchar_mask_next |= 1 << i;

	return 0;
}

int plcdd_customchar_alloc2(struct plcdd_display *display)
{
	char chars_free = ~(display->customchar_mask_curr | display->customchar_mask_next);
	if (!chars_free)
	{
		// we can't avoid a glitch
		chars_free = display->customchar_mask_next;
		if (!chars_free) return 1;
	}

	int i = ffs(chars_free) - 1;
	if (i < 0 || i >= 8) return '?';

	display->customchar_mask_next |= 1 << i;

	return i != 0 ? i : '\x0B';
}

int plcdd_customchar_alloc(struct plcdd_display *display, const char def[8])
{
	int i = plcdd_customchar_alloc2(display);

	if (plcdd_customchar_define(display, i, def))
	{
		return '!';
	}

	return i;
}

int plcdd_customchar_free(struct plcdd_display *display, int i)
{
	if (i == '\x0B') i = 0;
	if (i < 0 || i >= 8) return 1;

	if (!(display->customchar_mask_next & (1 << i)))
	{
		return 1;
	}

	display->customchar_mask_next &= ~(1 << i);

	return 0;
}

void plcdd_customchar_from_asciiart(char def[8], const char *p)
{
	const char *def_end = &def[8];
	char curr = 0;
	for (; *p && def != def_end; p++)
	{
		if (*p == '\n')
		{
			*def++ = curr;
			curr = 0;
		}
		else if (isspace(*p))
		{
			curr = (curr << 1) | 0;
		}
		else
		{
			curr = (curr << 1) | 1;
		}
	}

	while (def != def_end)
	{
		*def++ = curr;
		curr = 0;
	}
}
