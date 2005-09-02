/*
** pork_event.c - /event commands
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
#include <pork_events.h>
#include <pork_screen_io.h>
#include <pork_command.h>

USER_COMMAND(cmd_event_add) {
	char *event_type;
	u_int32_t refnum;
	struct event *events = acct->events;

	if (args == NULL || *args == '\0') {
		event_list(events, NULL);
		return;
	}

	event_type = strsep(&args, " ");
	if (event_type == NULL) {
		event_list(events, NULL);
		return;
	}

	strtoupper(event_type);
	if (args == NULL) {
		event_list(events, event_type);
		return;
	}

	if (event_add(events, event_type, args, &refnum) != 0)
		screen_err_msg("Error adding handler for %s", event_type);
	else {
		screen_cmd_output("Event handler %s installed for %s (refnum %u)",
			args, event_type, refnum);
	}
}

USER_COMMAND(cmd_event_del) {
	char *event_type;
	int ret;

	if (args == NULL)
		return;

	event_type = strsep(&args, " ");
	if (event_type == NULL)
		return;

	ret = event_del_type(acct->events, event_type, args);
	if (ret == 0) {
		if (args == NULL) {
			screen_cmd_output("Successfully removed handler %s for %s",
				event_type, args);
		} else
			screen_cmd_output("Successfully removed all handlers for %s", args);
	} else {
		if (args == NULL) {
			screen_err_msg("Error removing handler %s for %s",
				event_type, args);
		} else
			screen_err_msg("Error removing all handlers for %s", args);
	}
}

USER_COMMAND(cmd_event_del_refnum) {
	u_int32_t refnum;

	if (args == NULL)
		return;

	if (str_to_uint(args, &refnum) != 0) {
		screen_err_msg("Invalid event refnum: %s", args);
		return;
	}

	if (event_del_refnum(acct->events, refnum) != 0)
		screen_err_msg("Error deleting event refnum %s", args);
	else
		screen_cmd_output("Event refnum %s was removed", args);
}

USER_COMMAND(cmd_event_list) {
	event_list(acct->events, args);
}

USER_COMMAND(cmd_event_purge) {
	event_purge(acct->events);
}

struct command event_command[] = {
	{ "add",			cmd_event_add			},
	{ "del",			cmd_event_del			},
	{ "del_refnum",		cmd_event_del_refnum	},
	{ "list",			cmd_event_list			},
	{ "purge",			cmd_event_purge			},
};
