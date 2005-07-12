/*
** pork_irc_cmd.c - IRC specific user commands.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_queue.h>
#include <pork_imsg.h>
#include <pork_io.h>
#include <pork_misc.h>
#include <pork_color.h>
#include <pork_html.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_command.h>
#include <pork_conf.h>
#include <pork_chat.h>
#include <pork_set.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

#include <pork_irc.h>
#include <pork_irc_cmd.h>
#include <pork_irc_set.h>

/* XXX - todo */

static USER_COMMAND(irc_cmd_save) {
	/* save default */
}

static USER_COMMAND(irc_cmd_set) {
	if (acct == NULL)
		proto_set(proto_get(PROTO_IRC), NULL, args);
	else
		proto_set(acct->proto, acct->proto_prefs, args);
}

static USER_COMMAND(irc_cmd_ctcp) {
	char *dest;

	dest = strsep(&args, " ");
	if (dest == NULL || args == NULL)
		return;

	irc_ctcp(acct, dest, args);
}

static USER_COMMAND(irc_cmd_mode) {
	if (args != NULL)
		irc_mode(acct, args);
}

static USER_COMMAND(irc_cmd_quote) {
	if (args != NULL)
		irc_quote(acct, args);
}

static USER_COMMAND(irc_cmd_who) {
	struct imwindow *win = cur_window();
	if (args != NULL)
		irc_who(acct, args);

	if (args == NULL && win->type == WIN_TYPE_CHAT) {
		if (win->data != NULL) {
			struct chatroom *chat = win->data;

			chat_who(acct, chat->title);
		}
	}
}

static USER_COMMAND(irc_cmd_whowas) {
	if (args != NULL)
		irc_whowas(acct, args);
}

static struct command irc_command[] = {
	{ "ctcp",				irc_cmd_ctcp	},
	{ "mode",				irc_cmd_mode	},
	{ "quote",				irc_cmd_quote	},
	{ "set",				irc_cmd_set		},
	{ "save",				irc_cmd_save	},
	{ "who",				irc_cmd_who		},
	{ "whowas",				irc_cmd_whowas	},
};

void irc_cmd_setup(struct pork_proto *proto) {
	proto->cmd = irc_command;
	proto->num_cmds = array_elem(irc_command);
}
