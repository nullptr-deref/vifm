/* vifm
 * Copyright (C) 2001 Ken Steen.
 * Copyright (C) 2011 xaizek.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "bookmarks.h"

#include <sys/types.h>

#include <ctype.h> /* isalnum() */
#include <stddef.h> /* NULL */
#include <string.h>

#include "cfg/config.h"
#include "utils/fs.h"
#include "utils/macros.h"
#include "utils/str.h"
#include "filelist.h"
#include "status.h"
#include "ui.h"

static void free_bookmark(const int bmark_index);
static int mark2index(const char mark);
static int move_to_bookmark(FileView *view, const char mark);
static int is_bmark_by_index_empty(const int bmark_index);

bookmark_t bookmarks[NUM_BOOKMARKS];

const char valid_bookmarks[] =
{
	'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '<', '>',
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
	'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
	'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
	'\0'
};
ARRAY_GUARD(valid_bookmarks, NUM_BOOKMARKS + 1);

/* List of special bookmarks that can't be set manually, hence require special
 * treating in some cases. */
static const char spec_bookmarks[] =
{
	'<', '>', '\'',
	'\0'
};

char
index2mark(const int bmark_index)
{
	if(bmark_index >= 0 && bmark_index < ARRAY_LEN(valid_bookmarks) - 1)
	{
		return valid_bookmarks[bmark_index];
	}
	return '\0';
}

int
is_valid_bookmark(const int bmark_index)
{
	if(bmark_index < 0 || bmark_index >= ARRAY_LEN(bookmarks))
	{
		return 0;
	}
	/* the bookmark is valid if the file and the directory exists */
	else if(is_bmark_by_index_empty(bmark_index))
	{
		return 0;
	}
	else
	{
		return is_valid_dir(bookmarks[bmark_index].directory);
	}
}

int
is_bookmark_empty(const char mark)
{
	const int bmark_index = mark2index(mark);
	return is_bmark_by_index_empty(bmark_index);
}

int
is_spec_bookmark(const int x)
{
	const char mark = index2mark(x);
	return char_is_one_of(spec_bookmarks, mark);
}

void
remove_bookmark(const int mark)
{
	const int bmark_index = mark2index(mark);
	if(!is_bmark_by_index_empty(bmark_index))
	{
		free_bookmark(bmark_index);
	}
}

void
remove_all_bookmarks(void)
{
	const char *p = valid_bookmarks;
	while(*p != '\0')
	{
		const int bmark_index = mark2index(*p++);
		remove_bookmark(bmark_index);
	}
}

static void
add_mark(const char mark, const char directory[], const char file[])
{
	const int bmark_index = mark2index(mark);

	/* In case the mark is already being used.  Free pointers first! */
	free_bookmark(bmark_index);

	bookmarks[bmark_index].directory = strdup(directory);
	bookmarks[bmark_index].file = strdup(file);
}

/* Frees memory allocated for bookmark with given index. */
static void
free_bookmark(const int bmark_index)
{
	free(bookmarks[bmark_index].directory);
	bookmarks[bmark_index].directory = NULL;

	free(bookmarks[bmark_index].file);
	bookmarks[bmark_index].file = NULL;
}

int
add_bookmark(const char mark, const char *directory, const char *file)
{
	if(!char_is_one_of(valid_bookmarks, mark) ||
			char_is_one_of(spec_bookmarks, mark))
	{
		status_bar_message("Invalid mark name");
		return 1;
	}

	add_mark(mark, directory, file);
	return 0;
}

void
set_specmark(const char mark, const char *directory, const char *file)
{
	if(char_is_one_of(spec_bookmarks, mark))
	{
		add_mark(mark, directory, file);
	}
}

int
check_mark_directory(FileView *view, char mark)
{
	int x = mark2index(mark);

	if(bookmarks[x].directory == NULL)
		return -1;

	if(stroscmp(view->curr_dir, bookmarks[x].directory) == 0)
		return find_file_pos_in_list(view, bookmarks[x].file);

	return -1;
}

/* Transforms a mark to an index.  Returns the index or -1 for invalid name of a
 * mark. */
static int
mark2index(const char mark)
{
	const char *pos = strchr(valid_bookmarks, mark);
	return (pos == NULL) ? -1 : (pos - valid_bookmarks);
}

int
get_bookmark(FileView *view, char key)
{
	switch(key)
	{
		case '\'':
			navigate_to(view, view->last_dir);
			return 0;
		case 27: /* ascii Escape */
		case 3: /* ascii ctrl c */
			move_to_list_pos(view, view->list_pos);
			return 0;

		default:
			return move_to_bookmark(view, key);
	}
}

/* Navigates the view to given mark if it's valid.  Returns new value for
 * save_msg flag. */
static int
move_to_bookmark(FileView *view, char mark)
{
	int bmark_index = mark2index(mark);

	if(bmark_index != -1 && is_valid_bookmark(bmark_index))
	{
		if(change_directory(view, bookmarks[bmark_index].directory) >= 0)
		{
			load_dir_list(view, 1);
			(void)ensure_file_is_selected(view, bookmarks[bmark_index].file);
		}
	}
	else
	{
		if(!char_is_one_of(valid_bookmarks, mark))
			status_bar_message("Invalid mark name");
		else if(is_bmark_by_index_empty(bmark_index))
			status_bar_message("Mark is not set");
		else
			status_bar_message("Mark is invalid");

		move_to_list_pos(view, view->list_pos);
		return 1;
	}
	return 0;
}

int
init_active_bookmarks(const char marks[], int active_bookmarks[])
{
	int i, x;

	i = 0;
	for(x = 0; x < NUM_BOOKMARKS; ++x)
	{
		if(is_bmark_by_index_empty(x))
			continue;
		if(!char_is_one_of(marks, index2mark(x)))
			continue;
		active_bookmarks[i++] = x;
	}
	return i;
}

/* Checks whether bookmark specified by its index is empty.  Returns non-zero if
 * so, otherwise zero is returned. */
static int
is_bmark_by_index_empty(const int bmark_index)
{
	if(bmark_index < 0 || bmark_index >= ARRAY_LEN(bookmarks))
	{
		return 1;
	}

	/* Checking both is a bit paranoid, one should be enough. */
	return bookmarks[bmark_index].directory == NULL ||
			bookmarks[bmark_index].file == NULL;
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 : */
