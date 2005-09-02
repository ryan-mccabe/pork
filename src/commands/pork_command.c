/*
** pork_command.c - interface to commands typed by the user
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
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

struct command_set command_set[] = {
	{	command,			array_elem(command),			""			},
	{	window_command,		array_elem(window_command),		"win "		},
	{	history_command,	array_elem(history_command),	"history "	},
	{	input_command,		array_elem(input_command),		"input "	},
	{	scroll_command,		array_elem(scroll_command),		"scroll "	},
	{	buddy_command,		array_elem(buddy_command),		"buddy "	},
	{	blist_command,		array_elem(blist_command),		"blist "	},
	{	timer_command,		array_elem(timer_command),		"timer "	},
	{	event_command,		array_elem(event_command),		"event "	},
	{	chat_command,		array_elem(chat_command),		"chat "		},
	{	file_command,		array_elem(file_command),		"file "		},
	{	acct_command,		array_elem(acct_command),		"acct "		},
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

		ret = alias_resolve(&screen.alias_hash, str, &alias_str);
		if (ret == 0)
			str = alias_str;
		else if (ret == 1) {
			screen_err_msg("The alias chain is too long");
			return (-1);
		}
	}

	cmd_str = strsep(&str, " \t");

	cmd = bsearch(cmd_str, command_set[set].set, command_set[set].elem,
			sizeof(struct command), cmd_compare);

	if (cmd == NULL) {
		struct pork_proto *proto;

		if (set == CMDSET_MAIN && (proto = proto_get_name(cmd_str)) != NULL) {
			cmd_str = strsep(&str, " \t");

			if (cmd_str != NULL && cmd_str[0] != '\0') {
				cmd = bsearch(cmd_str, proto->cmd, proto->num_cmds,
						sizeof(struct command), cmd_compare);

				if (cmd == NULL) {
					screen_err_msg("Unknown %s command: %s",
						proto->name, cmd_str);
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
						screen_err_msg("%s may not run %s commands",
							acct->username, proto->name);
						return (-1);
					}
				}
			} else {
				if (str != NULL)
					screen_err_msg("Unknown %s command: %s", proto->name, str);

				return (-1);
			}
		} else {
			screen_err_msg("Unknown %scommand: %s",
				command_set[set].type, cmd_str);
			return (-1);
		}
	}

	cmd->cmd(acct, str);
	return (0);
}

inline int run_command(struct pork_acct *acct, char *str) {
	return (run_one_command(acct, str, CMDSET_MAIN));
}

int run_mcommand(struct pork_acct *acct, char *str) {
	int i = 0;
	char *copystr = xstrdup(str);
	char *cmdstr = copystr;
	char *curcmd;

	curcmd = strsep(&cmdstr, ";");
	if (curcmd == NULL)
		i = run_one_command(acct, cmdstr, CMDSET_MAIN);
	else {
		while (curcmd != NULL && i != -1) {
			char cmdchars = opt_get_char(screen.global_prefs, OPT_CMDCHARS);

			while (*curcmd == ' ')
				curcmd++;

			while (*curcmd == cmdchars)
				curcmd++;

			i = run_one_command(acct, curcmd, CMDSET_MAIN);
			curcmd = strsep(&cmdstr, ";");
		}
	}

	free(copystr);
	return (i);
}

static int command_send_to_win(struct pork_acct *acct, char *str) {
	struct imwindow *imwindow = cur_window();

	if (str == NULL || !acct->connected)
		return (-1);

	if (imwindow->type == WIN_TYPE_PRIVMSG)
		pork_msg_send(acct, imwindow->target, str);
	else if (imwindow->type == WIN_TYPE_CHAT) {
		struct chatroom *chat = imwindow->data;

		if (chat == NULL) {
			screen_err_msg("%s is not a member of %s",
				acct->username, imwindow->target);
		} else
			chat_send_msg(acct, chat, chat->title, str);
	}
}

int command_enter_str(struct pork_acct *acct, char *str) {
	if (!event_generate(acct->events, EVENT_SEND_LINE,
		str, acct->refnum))
	{
		return (-1);
	}

	if (str[0] == opt_get_char(screen.global_prefs, OPT_CMDCHARS))
		run_command(acct, &str[1]);
	else
		command_send_to_win(acct, str);
}

