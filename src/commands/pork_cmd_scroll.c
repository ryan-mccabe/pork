/*
** pork_cmd_scroll.c - /scroll commands.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_input.h>
#include <pork_set.h>
#include <pork_input_set.h>
#include <pork_bind.h>
#include <pork_imsg.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_command.h>

USER_COMMAND(cmd_scroll_by) {
	int lines;

	if (args == NULL)
		return;

	if (str_to_int(args, &lines) != 0) {
		screen_err_msg("Invalid number of lines: %s", args);
		return;
	}

	imwindow_scroll_by(cur_window(), lines);
}

USER_COMMAND(cmd_scroll_down) {
	imwindow_scroll_down(cur_window());
}

USER_COMMAND(cmd_scroll_end) {
	imwindow_scroll_end(cur_window());
}

USER_COMMAND(cmd_scroll_pgdown) {
	imwindow_scroll_page_down(cur_window());
}

USER_COMMAND(cmd_scroll_pgup) {
	imwindow_scroll_page_up(cur_window());
}

USER_COMMAND(cmd_scroll_start) {
	imwindow_scroll_start(cur_window());
}

USER_COMMAND(cmd_scroll_up) {
	imwindow_scroll_up(cur_window());
}

static struct command scroll_command[] = {
	{ "by",				cmd_scroll_by			},
	{ "down",			cmd_scroll_down			},
	{ "end",			cmd_scroll_end			},
	{ "page_down",		cmd_scroll_pgdown		},
	{ "page_up",		cmd_scroll_pgup			},
	{ "start",			cmd_scroll_start		},
	{ "up",				cmd_scroll_up			},
};

struct command_set scroll_set = { scroll_command, array_elem(scroll_command), "scroll " };
