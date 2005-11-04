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
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_command.h>
#include <pork_conf.h>
#include <pork_chat.h>
#include <pork_set.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

#include <pork_irc.h>
#include <pork_irc_cmd.h>
#include <pork_irc_set.h>

USER_COMMAND(irc_cmd_set) {
	if (acct == NULL)
		proto_set(proto_get(PROTO_IRC), NULL, args);
	else
		proto_set(acct->proto, acct->proto_prefs, args);
}

USER_COMMAND(irc_cmd_ctcp) {
	char *dest;

	dest = strsep(&args, " ");
	if (dest == NULL || args == NULL)
		return;

	irc_ctcp(acct, dest, args);
}

USER_COMMAND(irc_cmd_mode) {
	if (args != NULL)
		irc_mode(acct, args);
}

USER_COMMAND(irc_cmd_quote) {
	if (args != NULL)
		irc_quote(acct, args);
}

USER_COMMAND(irc_cmd_who) {
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

USER_COMMAND(irc_cmd_whowas) {
	if (args != NULL)
		irc_whowas(acct, args);
}

/* This needs to save either user preferences or defaults, or both. */
USER_COMMAND(irc_cmd_save) {
}

USER_COMMAND(FIXME) {
	screen_err_msg("This command is not implemented yet.");
}

static struct command irc_command[] = {
	{ "capab",				FIXME			},
	{ "close",				FIXME			},
	{ "connect",			FIXME			},
	{ "ctcp",				irc_cmd_ctcp	},
	{ "die",				FIXME			},
	{ "dline",				FIXME			},
	{ "hash",				FIXME			},
	{ "ison",				FIXME			},
	{ "kill",				FIXME			},
	{ "kline",				FIXME			},
	{ "knock",				FIXME			},
	{ "gline",				FIXME			},
	{ "links",				FIXME			},
	{ "list",				FIXME			},
	{ "locops",				FIXME			},
	{ "ltrace",				FIXME			},
	{ "lusers",				FIXME			},
	{ "lwallops",			FIXME			},
	{ "mode",				irc_cmd_mode	},
	{ "motd",				FIXME			},
	{ "names",				FIXME			},
	{ "operwall",			FIXME			},
	{ "pass",				FIXME			},
	{ "quote",				irc_cmd_quote	},
	{ "rehash",				FIXME			},
	{ "restart",			FIXME			},
	{ "save",				irc_cmd_save	},
	{ "server",				FIXME			},
	{ "set",				irc_cmd_set		},
	{ "sete",				FIXME			},
	{ "sjoin",				FIXME			},
	{ "squit",				FIXME			},
	{ "stats",				FIXME			},
	{ "svinfo",				FIXME			},
	{ "time",				FIXME			},
	{ "testline",			FIXME			},
	{ "trace",				FIXME			},
	{ "undline",			FIXME			},
	{ "ungline",			FIXME			},
	{ "unkline",			FIXME			},
	{ "user",				FIXME			},
	{ "userhost",			FIXME			},
	{ "users",				FIXME			},
	{ "version",			FIXME			},
	{ "wallops",			FIXME			},
	{ "wii",				FIXME			},
	{ "who",				irc_cmd_who		},
	{ "whowas",				irc_cmd_whowas	},
};

void irc_cmd_setup(struct pork_proto *proto) {
	proto->cmd = irc_command;
	proto->num_cmds = array_elem(irc_command);
}
