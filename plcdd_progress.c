#include <stdlib.h>
#include <string.h>

#include "plcdd_display.h"
#include "plcdd_customchar.h"
#include "plcdd_window.h"

#include "plcdd_progress.h"

#define PLCDD_PROGRESS_CHARS_TOTAL 11

static const char *plcdd_progress_defs[PLCDD_PROGRESS_CHARS_TOTAL] =
{
	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"#    \n",

	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"#    \n"
	"##   \n",

	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"#    \n"
	"##   \n"
	"###  \n",

	"     \n"
	"     \n"
	"     \n"
	"     \n"
	"#    \n"
	"##   \n"
	"###  \n"
	"#### \n",

	"     \n"
	"     \n"
	"     \n"
	"#    \n"
	"##   \n"
	"###  \n"
	"#### \n"
	"#####\n",

	"     \n"
	"     \n"
	"#    \n"
	"##   \n"
	"###  \n"
	"#### \n"
	"#####\n"
	"#####\n",

	"     \n"
	"#    \n"
	"##   \n"
	"###  \n"
	"#### \n"
	"#####\n"
	"#####\n"
	"#####\n",

	"#    \n"
	"##   \n"
	"###  \n"
	"#### \n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n",

	"##   \n"
	"###  \n"
	"#### \n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n",

	"###  \n"
	"#### \n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n",

	"#### \n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n"
	"#####\n",
};

static char plcdd_progress_defs_bin[PLCDD_PROGRESS_CHARS_TOTAL][8];

static const char *plcdd_progress_def_on =
"#####\n"
"#####\n"
"#####\n"
"#####\n"
"#####\n"
"#####\n"
"#####\n"
"#####\n";

static char plcdd_progress_char_off   = ' ';
static char plcdd_progress_char_on    = '#';

void plcdd_progress_init(struct plcdd_display *display)
{
	plcdd_progress_char_off = ' ';

	char char_def[8];
	plcdd_customchar_from_asciiart(char_def, plcdd_progress_def_on);
	plcdd_progress_char_on = plcdd_customchar_alloc(display, char_def);

	for (size_t i = 0; i < PLCDD_PROGRESS_CHARS_TOTAL; i++)
	{
		plcdd_customchar_from_asciiart(plcdd_progress_defs_bin[i], plcdd_progress_defs[i]);
	}
}

struct plcdd_progress *plcdd_progress_new_at(struct plcdd_progress *progress, struct plcdd_display *display, unsigned int y, unsigned int x, unsigned int width, size_t len)
{
	plcdd_window_new_at(&progress->window, display, y, x, width, len);
	progress->window.dispoff = 0;

	progress->char1 = '1';
	progress->char2 = '2';

	progress->curr = 0;

	memset(progress->window.buf, plcdd_progress_char_off, progress->window.len);

	return progress;
}

void plcdd_progress_dtor(struct plcdd_progress *progress)
{
	plcdd_window_dtor(&progress->window);
}

struct plcdd_progress *plcdd_progress_new(struct plcdd_display *display, unsigned int y, unsigned int x, unsigned int width, size_t len)
{
	struct plcdd_progress *progress = malloc(sizeof(*progress));

	return plcdd_progress_new_at(progress, display, y, x, width, len);
}

void plcdd_progress_free(struct plcdd_progress *progress)
{
	plcdd_progress_dtor(progress);

	free(progress);
}

void plcdd_progress_draw(struct plcdd_progress *progress)
{
	plcdd_window_draw(&progress->window);
}

int plcdd_progress_max(struct plcdd_progress *progress)
{
	return (progress->window.len + 1) * PLCDD_PROGRESS_CHARS_N + 1;
}

int plcdd_progress_set(struct plcdd_progress *progress, int new)
{
	int max = plcdd_progress_max(progress);
	// limit to bounds
	if (new <  0  ) new = 0;
	if (new >= max) new = max - 1;

	int old = progress->curr;

	if (new == old) return 0;

	unsigned int major = new / PLCDD_PROGRESS_CHARS_N;
	unsigned int minor = new % PLCDD_PROGRESS_CHARS_N;

	unsigned int major_old = old / PLCDD_PROGRESS_CHARS_N;

	if (major > major_old)
	{
		for (unsigned int x = major_old >= 1 ? major_old - 1 : 0; x < major - 1; x++)
		{
			progress->window.buf[x] = plcdd_progress_char_on;
		}
	}
	else if (major < major_old)
	{
		for (unsigned int x = major_old < progress->window.len ? major_old : progress->window.len - 1; x > major; x--)
		{
			progress->window.buf[x] = plcdd_progress_char_off;
		}
	}

		if (major != major_old)
		{
			if (major == major_old + 1)
			{
				plcdd_customchar_free(progress->window.display, progress->char1);
				progress->char1 = progress->char2;
				progress->char2 = '2';
			}
			else if (major == major_old - 1)
			{
				plcdd_customchar_free(progress->window.display, progress->char2);
				progress->char2 = progress->char1;
				progress->char1 = '1';
			}
			else
			{
				plcdd_customchar_free(progress->window.display, progress->char1);
				progress->char1 = '1';
				plcdd_customchar_free(progress->window.display, progress->char2);
				progress->char2 = '2';
			}
		}


	if (major >= 1)
	{
		if (progress->char1 > '\x0B')
		{
			progress->char1 = plcdd_customchar_alloc2(progress->window.display);
		}

		plcdd_customchar_define(progress->window.display, progress->char1, plcdd_progress_defs_bin[minor + 5]);
		progress->window.buf[major - 1] = progress->char1;
	}
	else
	{
		plcdd_customchar_free(progress->window.display, progress->char1);
		progress->char1 = '1';
	}

	if (minor != 0 && major < progress->window.len)
	{
		if (progress->char2 > '\x0B')
		{
			progress->char2 = plcdd_customchar_alloc2(progress->window.display);
		}

		plcdd_customchar_define(progress->window.display, progress->char2, plcdd_progress_defs_bin[minor - 1]);
		progress->window.buf[major    ] = progress->char2;
	}
	else
	{
		if (major < progress->window.len)
		{
			progress->window.buf[major    ] = plcdd_progress_char_off;
		}

		plcdd_customchar_free(progress->window.display, progress->char2);
		progress->char2 = '2';
	}

	progress->curr = new;

	return 0;
}

