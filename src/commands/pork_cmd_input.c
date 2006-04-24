/*
** pork_cmd_input.c - /input commands.
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
#include <pork_set.h>
#include <pork_input_set.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_imsg.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_command.h>

USER_COMMAND(cmd_input_bkspace) {
	input_bkspace(cur_window()->input);
}

USER_COMMAND(cmd_input_clear) {
	input_clear_line(cur_window()->input);
}

USER_COMMAND(cmd_input_clear_prev) {
	input_clear_prev_word(cur_window()->input);
}

USER_COMMAND(cmd_input_clear_next) {
	input_clear_next_word(cur_window()->input);
}

USER_COMMAND(cmd_input_clear_to_end) {
	input_clear_to_end(cur_window()->input);
}

USER_COMMAND(cmd_input_focus_next) {
	struct imwindow *win = cur_window();

	imwindow_switch_focus(win);
	imwindow_blist_draw(win);
}

USER_COMMAND(cmd_input_clear_to_start) {
	input_clear_to_start(cur_window()->input);
}

USER_COMMAND(cmd_input_delete) {
	input_delete(cur_window()->input);
}

USER_COMMAND(cmd_input_end) {
	input_end(cur_window()->input);
}

USER_COMMAND(cmd_input_insert) {
	if (args != NULL)
		input_insert_str(cur_window()->input, args);
}

USER_COMMAND(cmd_input_left) {
	input_move_left(cur_window()->input);
}

USER_COMMAND(cmd_input_prev_word) {
	input_prev_word(cur_window()->input);
}

USER_COMMAND(cmd_input_prompt) {
	if (blank_str(args) ||
		!strcasecmp(args, "off") || !strcasecmp(args, "false"))
	{
		input_set_prompt(cur_window()->input, NULL);
		screen_cmd_output(_("Input prompt set to off"));
	} else if (	args != NULL &&
				(!strcasecmp(args, "on") || !strcasecmp(args, "true")))
	{
		char *prompt;

		prompt = opt_get_str(cur_window()->input->prefs, INPUT_OPT_PROMPT_STR);
		if (prompt != NULL)
			input_set_prompt(cur_window()->input, prompt);
		else
			screen_err_msg(_("No prompt format has been specified"));
	}
}

USER_COMMAND(cmd_input_next_word) {
	input_next_word(cur_window()->input);
}

USER_COMMAND(cmd_input_remove) {
	if (!blank_str(args)) {
		int len;

		if (str_to_int(args, &len) == -1)
			screen_err_msg(_("Bad number of characters: %s"), args);
		else
			input_remove(cur_window()->input, len);
	}
}

USER_COMMAND(cmd_input_right) {
	input_move_right(cur_window()->input);
}

USER_COMMAND(cmd_input_send) {
	input_send(acct, cur_window()->input, args);
}

USER_COMMAND(cmd_input_set) {
	struct pork_input *input;
	struct pref_val *pref;

	if (blank_str(args)) {
		input = cur_window()->input;
		pref = input->prefs;
	} else if (!strncasecmp(args, "-default", 8)) {
		args += 8;
		while (args[0] == ' ')
			args++;

		input = NULL;
		pref = input_get_default_prefs();
	} else {
		input = cur_window()->input;
		pref = input->prefs;
	}

	opt_set_var(pref, args, input);
}

USER_COMMAND(cmd_input_start) {
	input_home(cur_window()->input);
}

static struct command input_command[] = {
	{ "backspace",				cmd_input_bkspace			},
	{ "clear",					cmd_input_clear				},
	{ "clear_next_word",		cmd_input_clear_next		},
	{ "clear_prev_word",		cmd_input_clear_prev		},
	{ "clear_to_end",			cmd_input_clear_to_end		},
	{ "clear_to_start",			cmd_input_clear_to_start	},
	{ "delete",					cmd_input_delete			},
	{ "end",					cmd_input_end				},
	{ "focus_next",				cmd_input_focus_next		},
	{ "insert",					cmd_input_insert			},
	{ "left",					cmd_input_left				},
	{ "next_word",				cmd_input_next_word			},
	{ "prev_word",				cmd_input_prev_word			},
	{ "prompt",					cmd_input_prompt			},
	{ "remove",					cmd_input_remove			},
	{ "right",					cmd_input_right				},
	{ "send",					cmd_input_send				},
	{ "set",					cmd_input_set				},
	{ "start",					cmd_input_start				},
};

struct command_set input_set = { input_command, array_elem(input_command), "input " };
