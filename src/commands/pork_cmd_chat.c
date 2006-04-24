/*
** pork_chat.c - /chat commands.
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
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_imsg.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_buddy.h>
#include <pork_chat.h>
#include <pork_msg.h>
#include <pork_command.h>

USER_COMMAND(cmd_chat_ban) {
	struct imwindow *win = cur_window();
	struct chatroom *chat;
	char *arg1;
	char *arg2;

	if (blank_str(args))
		return;

	arg1 = strsep(&args, " ");

	chat = chat_find(acct, arg1);
	if (chat == NULL) {
		if (win->type == WIN_TYPE_CHAT && win->data != NULL)
			chat_ban(acct, win->data, arg1);
		else
			screen_err_msg(_("%s is not a member of %s"), acct->username, arg1);

		return;
	}

	arg2 = strsep(&args, " ");
	if (arg2 != NULL)
		chat_ban(acct, chat, arg2);
}

USER_COMMAND(cmd_chat_ignore) {
	struct imwindow *imwindow = cur_window();
	char *chat_name;
	char *user_name;

	if (blank_str(args))
		return;

	chat_name = strsep(&args, " ");
	user_name = args;

	if (user_name == NULL) {
		struct chatroom *chat = imwindow->data;

		if (imwindow->type != WIN_TYPE_CHAT || chat == NULL) {
			screen_err_msg(_("You must specify a chat room if the current window is not a chat window"));
			return;
		}

		user_name = chat_name;
		chat_name = chat->title;
	}

	chat_ignore(acct, chat_name, user_name);
}

USER_COMMAND(cmd_chat_invite) {
	struct imwindow *imwindow = cur_window();
	char *chat_name;
	char *user_name;
	char *invite_msg;

	if (blank_str(args))
		return;

	chat_name = strsep(&args, " ");
	user_name = strsep(&args, " ");
	invite_msg = args;

	if (user_name == NULL) {
		struct chatroom *chat = imwindow->data;

		if (imwindow->type != WIN_TYPE_CHAT || chat == NULL) {
			screen_err_msg(_("You must specify a chat room if the current window is not a chat window"));
			return;
		}

		user_name = chat_name;
		chat_name = chat->title;
	}

	chat_invite(acct, chat_name, user_name, invite_msg);
}

USER_COMMAND(cmd_chat_join) {
	chat_join(acct, args);
}

USER_COMMAND(cmd_chat_kick) {
	struct imwindow *win = cur_window();
	struct chatroom *chat;
	char *arg1;
	char *arg2;

	if (blank_str(args))
		return;

	arg1 = strsep(&args, " ");

	chat = chat_find(acct, arg1);
	if (chat == NULL) {
		if (win->type == WIN_TYPE_CHAT && win->data != NULL)
			chat_kick(acct, win->data, arg1, args);
		else
			screen_err_msg(_("%s is not a member of %s"), acct->username, arg1);

		return;
	}

	arg2 = strsep(&args, " ");
	if (arg2 != NULL)
		chat_kick(acct, chat, arg2, args);
}

USER_COMMAND(cmd_chat_leave) {
	struct imwindow *win = cur_window();
	char *name = args;

	if (blank_str(name)) {
		struct chatroom *chat;

		if (win->type != WIN_TYPE_CHAT) {
			screen_err_msg(_("You must specify a chat room if the current window is not a chat window"));
			return;
		}

		if (win->data == NULL)
			return;

		chat = win->data;
		name = chat->title;
	}

	chat_leave(acct, name, 1);
}

USER_COMMAND(cmd_chat_list) {
	chat_list(acct);
}

USER_COMMAND(cmd_chat_send) {
	struct imwindow *win;
	char *chat_name;

	if (blank_str(args))
		return;

	chat_name = strsep(&args, " ");
	if (chat_name == NULL || args == NULL) {
		screen_err_msg(_("You must specify a chatroom and a message"));
		return;
	}

	win = imwindow_find_chat_target(acct, chat_name);
	if (win == NULL || win->data == NULL) {
		screen_err_msg(_("%s is not joined to %s"), acct->username, chat_name);
		return;
	}

	chat_send_msg(acct, win->data, chat_name, args);
}

USER_COMMAND(cmd_chat_topic) {
	struct imwindow *win = cur_window();
	char *topic = NULL;
	struct chatroom *chat = NULL;

	if (args != NULL) {
		topic = strchr(args, ' ');
		if (topic != NULL)
			*topic++ = '\0';

		chat = chat_find(acct, args);
	}

	if (chat == NULL) {
		if (topic != NULL)
			topic[-1] = ' ';

		topic = args;

		if (win->type == WIN_TYPE_CHAT)
			chat = win->data;
		else {
			screen_err_msg(_("You must specify a chat room if the current window isn't a chat window"));
			return;
		}
	}

	chat_set_topic(acct, chat, topic);
}

USER_COMMAND(cmd_chat_unignore) {
	struct imwindow *imwindow = cur_window();
	char *chat_name;
	char *user_name;

	if (blank_str(args))
		return;

	chat_name = strsep(&args, " ");
	user_name = args;

	if (user_name == NULL) {
		struct chatroom *chat = imwindow->data;

		if (imwindow->type != WIN_TYPE_CHAT || chat == NULL) {
			screen_err_msg(_("You must specify a chat room if the current window is not a chat window"));
			return;
		}

		user_name = chat_name;
		chat_name = chat->title;
	}

	chat_unignore(acct, chat_name, user_name);
}

USER_COMMAND(cmd_chat_who) {
	struct imwindow *imwindow = cur_window();

	if (blank_str(args)) {
		struct chatroom *chat;

		if (imwindow->type != WIN_TYPE_CHAT) {
			screen_err_msg(_("You must specify a chat room if the current window is not a chat window"));
			return;
		}

		chat = imwindow->data;
		args = chat->title;
	}

	chat_who(acct, args);
}

static struct command chat_command[] = {
	{ "ban",				cmd_chat_ban			},
	{ "ignore",				cmd_chat_ignore			},
	{ "invite",				cmd_chat_invite			},
	{ "join",				cmd_chat_join			},
	{ "kick",				cmd_chat_kick			},
	{ "leave",				cmd_chat_leave			},
	{ "list",				cmd_chat_list			},
	{ "send",				cmd_chat_send			},
	{ "topic",				cmd_chat_topic			},
	{ "unignore",			cmd_chat_unignore		},
	{ "who",				cmd_chat_who			},
};

struct command_set chat_set = { chat_command, array_elem(chat_command), "chat " };
