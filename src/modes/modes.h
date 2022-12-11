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

#ifndef VIFM__MODES__MODES_H__
#define VIFM__MODES__MODES_H__

#include <stddef.h>

enum
{
	NORMAL_MODE,
	CMDLINE_MODE,
	VISUAL_MODE,
	MENU_MODE,
	SORT_MODE,
	ATTR_MODE,
	CHANGE_MODE,
	VIEW_MODE,
	FILE_INFO_MODE,
	MSG_MODE,
	MORE_MODE,
	MODES_COUNT
};

/* Registers modes. */
void modes_init(void);

/* A hook-like function that performs mode-specific actions at the start of an
 * event loop iteration. */
void modes_pre(void);

/* Executes poll-based requests for any of the active modes. */
void modes_periodic(void);

/* A hook-like function that performs mode-specific actions at the end of an
 * event loop iteration. */
void modes_post(void);

/* Redraws currently active mode. */
void modes_redraw(void);

/* A lighter version of modes_redraw(). */
void modes_update(void);

/* Clears input bar or draws current input there depending on the mode. */
void modes_input_bar_update(const wchar_t str[]);

/* Clears input bar if it's being used by the current mode. */
void modes_input_bar_clear(void);

/* Returns non-zero if current mode is a menu like one. */
int modes_is_menu_like(void);

/* Checks if current mode is a dialog.  Returns non-zero if so. */
int modes_is_dialog_like(void);

/* Aborts one of menu-like modes if any of them is currently active. */
void modes_abort_menu_like(void);

/* Either prints appropriate message on the statusbar or clears it depending on
 * the mode and its state. */
void modes_statusbar_update(void);

#endif /* VIFM__MODES__MODES_H__ */

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 filetype=c : */
