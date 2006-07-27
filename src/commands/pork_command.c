/*
** pork_command.c - interface to commands typed by the user
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
** Copyright (C) 2002-2004 Amber Adams <amber@ojnk.net>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_alias.h>
#include <pork_imsg.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_msg.h>
#include <pork_chat.h>
#include <pork_events.h>
#include <pork_command.h>

extern struct command_set main_set;
extern struct command_set win_set;
extern struct command_set history_set;
extern struct command_set input_set;
extern struct command_set scroll_set;
extern struct command_set buddy_set;
extern struct command_set blist_set;
extern struct command_set timer_set;
extern struct command_set event_set;
extern struct command_set chat_set;
extern struct command_set file_set;
extern struct command_set acct_set;
extern struct command_set perl_set;

struct command_set *command_set[] = {
	&main_set,
	&win_set,
	&history_set,
	&input_set,
	&scroll_set,
	&buddy_set,
	&blist_set,
	&timer_set,
	&event_set,
	&chat_set,
	&file_set,
	&acct_set,
	&perl_set,
	NULL
};

static int cmd_compare(const void *l, const void *r) {
	char *key = (char *) l;
	struct command *cmd = (struct command *) r;

	return (strcasecmp(key, cmd->name));
}

int run_one_command(struct pork_acct *acct, char *str, u_int32_t set) {
	char *cmd_str;
	struct command *cmd;

	if (set == CMDSET_MAIN) {
		int ret;
		char *alias_str;

		ret = alias_resolve(&globals.alias_hash, str, &alias_str);
		if (ret == 0)
			str = alias_str;
		else if (ret == 1) {
			screen_err_msg(_("The alias chain is too long"));
			return (-1);
		}
	}

	cmd_str = strsep(&str, " \t");

	cmd = bsearch(cmd_str, command_set[set]->set, command_set[set]->elem,
			sizeof(struct command), cmd_compare);

	if (cmd == NULL) {
		int explicit = 0;
		struct pork_proto *proto;

		if (set != CMDSET_MAIN) {
			screen_err_msg(_("Unknown %scommand: %s"),
				command_set[set]->type, cmd_str);
			return (-1);
		}

		proto = proto_get_name(cmd_str);
		if (proto != NULL) {
			explicit = 1;
			cmd_str = strsep(&str, " \t");
		} else
			proto = cur_window()->owner->proto;

		if (proto != NULL && proto->protocol >= 0 &&
			cmd_str != NULL && cmd_str[0] != '\0')
		{
			cmd = bsearch(cmd_str, proto->cmd, proto->num_cmds,
					sizeof(struct command), cmd_compare);

			if (cmd == NULL) {
				if (explicit) {
					screen_err_msg(_("Unknown %s command: %s"),
						proto->name, cmd_str);
				} else
					screen_err_msg(_("Unknown command: %s"), cmd_str);

				return (-1);
			}

			if (proto != acct->proto) {
				/* yeah, this isn't a hack at all. */
				if (!strcasecmp(cmd->name, "set") ||
					!strcasecmp(cmd->name, "save"))
				{
					cmd->cmd(NULL, str);
					return (0);
				} else {
					screen_err_msg(_("%s may not run %s commands"),
						acct->username, proto->name);
					return (-1);
				}
			}
		} else {
			if (str != NULL)
				screen_err_msg(_("Unknown %s command: %s"), proto->name, str);
			else
				screen_err_msg(_("Unknown command: %s"), cmd_str);

			return (-1);
		}
	}

	cmd->cmd(acct, str);
	return (0);
}

inline int run_command(struct pork_acct *acct, char *str) {
	int ret;
	char *cmd = xstrdup(str);

	ret = run_one_command(acct, cmd, CMDSET_MAIN);
	free(cmd);
	return (ret);
}

static int command_send_to_win(struct pork_acct *acct, char *str) {
	struct imwindow *win = cur_window();

	if (str == NULL || !acct->connected)
		return (-1);

	if (win->type == WIN_TYPE_PRIVMSG)
		pork_msg_send(acct, win->target, str);
	else if (win->type == WIN_TYPE_CHAT) {
		struct chatroom *chat = win->data;

		if (chat == NULL) {
			screen_err_msg(_("%s is not a member of %s"),
				acct->username, win->target);
		} else
			chat_send_msg(acct, chat, chat->title, str);
	}

	return (0);
}

int command_enter_str(struct pork_acct *acct, char *str) {
	if (!event_generate(acct->events, EVENT_SEND_LINE,
		str, acct->refnum))
	{
		if (str[0] == opt_get_char(globals.prefs, OPT_CMDCHARS))
			run_command(acct, &str[1]);
		else
			command_send_to_win(acct, str);
	}

	return (0);
}
