/*
** pork_status.c - status bar implementation.
** Copyright (C) 2003-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_misc.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_cstr.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_status.h>
#include <pork_format.h>

int status_init(void) {
	WINDOW *win;

	win = newwin(STATUS_ROWS, globals.cols, globals.rows - STATUS_ROWS, 0);
	if (win == NULL)
		return (-1);

	nodelay(win, 1);
	set_default_win_opts(win);
	globals.status_bar = win;
	return (0);
}

/*
** Draw the status bar, using the format string OPT_FORMAT_STATUS. This
** parses the format string every time the status bar is redrawn, which
** could waste some cpu time, except that today's computers are sickeningly fast
** and the status bar isn't redrawn all that much anyway.
*/

void status_draw(struct pork_acct *acct) {
	char buf[1024];
	chtype status_bar[globals.cols + 1];
	struct imwindow *win = cur_window();
	int type;

	if (win->type == WIN_TYPE_CHAT)
		type = OPT_FORMAT_STATUS_CHAT;
	else
		type = OPT_FORMAT_STATUS;

	fill_format_str(type, buf, sizeof(buf), win, acct);
	format_apply_justification(buf, status_bar, array_elem(status_bar));

	mvwputstr(globals.status_bar, win->prefs, 0, 0, status_bar);
	wnoutrefresh(globals.status_bar);
}
