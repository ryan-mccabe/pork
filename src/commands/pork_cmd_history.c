/*
** pork_cmd_file.c - /history commands.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
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
#include <ncurses.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_set.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_imsg.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_command.h>

USER_COMMAND(cmd_history_clear) {
	input_history_clear(cur_window()->input);
}

USER_COMMAND(cmd_history_list) {
	struct imwindow *win = cur_window();
	struct pork_input *input = win->input;
	dlist_t *cur = input->history_end;
	u_int32_t i = 0;

	if (cur == NULL)
		return;

	screen_win_msg(win, 0, 0, 0, MSG_TYPE_CMD_OUTPUT, "Command history:");

	do {
		screen_win_msg(win, 0, 0, 0, MSG_TYPE_CMD_OUTPUT, "%u: %s", i++,
			(char *) cur->data);
		cur = cur->prev;
	} while (cur != NULL);
}

USER_COMMAND(cmd_history_next) {
	input_history_next(cur_window()->input);
}

USER_COMMAND(cmd_history_prev) {
	input_history_prev(cur_window()->input);
}

static struct command history_command[] = {
	{ "clear",		cmd_history_clear	},
	{ "list",		cmd_history_list	},
	{ "next",		cmd_history_next	},
	{ "prev",		cmd_history_prev	},
};

struct command_set history_set = { history_command, array_elem(history_command), "history " };
