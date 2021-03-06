/*
** pork_set_global.c - global /set command implementation.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_color.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

/*
** Name:		The name of the option
** Type:		The type of the option (boolean, string, int, char, color)
** Set func:	The function that checks and sets the requested value of the opt
** Change func:	The function called when the value of the option changes
*/

static struct pork_pref global_pref_list[] = {
	{	.name = "AUTOSAVE",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "BANNER",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "CMDCHARS",
		.type = OPT_TYPE_CHAR,
		.set = opt_set_char,
	},{	.name = "COLOR_BLIST_FOCUS",
		.type = OPT_TYPE_COLOR,
		.set = opt_set_color,
		.updated = pork_acct_update_blist_color,
	},{	.name = "COLOR_BLIST_NOFOCUS",
		.type = OPT_TYPE_COLOR,
		.set = opt_set_color,
		.updated = pork_acct_update_blist_color,
	},{	.name = "COLOR_BLIST_SELECTOR",
		.type = OPT_TYPE_COLOR,
		.set = opt_set_color,
		.updated = pork_acct_update_blist_color,
	},{	.name = "FORMAT_ACTION_RECV",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_ACTION_RECV_STATUS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_ACTION_SEND",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_ACTION_SEND_STATUS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_BLIST",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
		pork_acct_update_blist_format,
	},{	.name = "FORMAT_BLIST_GROUP",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "FORMAT_BLIST_IDLE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "FORMAT_BLIST_WARN",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "FORMAT_CHAT_CREATE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_IGNORE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_INVITE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_JOIN",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_KICK",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_LEAVE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_MODE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_NICK",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_QUIT",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_RECV",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_RECV_ACTION",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_RECV_NOTICE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_SEND",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_SEND_ACTION",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_SEND_NOTICE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_TOPIC",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CHAT_UNIGNORE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_CANCEL_LOCAL",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_CANCEL_REMOTE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_LOST",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_RECV_ACCEPT",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_RECV_ASK",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_RECV_COMPLETE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_RECV_RESUME",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_SEND_ACCEPT",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_SEND_ASK",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_SEND_COMPLETE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_FILE_SEND_RESUME",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_MSG_RECV",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_MSG_RECV_AUTO",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_MSG_RECV_STATUS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_MSG_SEND",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_MSG_SEND_AUTO",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_MSG_SEND_STATUS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_NOTICE_RECV",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_NOTICE_RECV_STATUS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_NOTICE_SEND",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_NOTICE_SEND_STATUS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_STATUS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_STATUS_ACTIVITY",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_STATUS_CHAT",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_STATUS_HELD",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_STATUS_IDLE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_STATUS_TIMESTAMP",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_STATUS_TYPING",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_STATUS_WARN",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_TIMESTAMP",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WARN_RECV",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WARN_RECV_AUTO",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WARN_SEND",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WARN_SEND_AUTO",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "PORK_DIR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "RECURSIVE_EVENTS",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "TEXT_BLIST_GROUP_COLLAPSED",
		.type = OPT_TYPE_STR,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "TEXT_BLIST_GROUP_EXPANDED",
		.type = OPT_TYPE_STR,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "TEXT_BUDDY_ACTIVE",
		.type = OPT_TYPE_STR,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "TEXT_BUDDY_AWAY",
		.type = OPT_TYPE_STR,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "TEXT_BUDDY_IDLE",
		.type = OPT_TYPE_STR,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "TEXT_BUDDY_WIRELESS",
		.type = OPT_TYPE_STR,
		.set = opt_set_format,
		.updated = pork_acct_update_blist_format,
	},{	.name = "TEXT_NO_NAME",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "TEXT_NO_ROOM",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "TEXT_TYPING",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "TEXT_TYPING_PAUSED",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	}
};

static const struct pref_set global_pref_set = {
	.num_opts = GLOBAL_NUM_OPTS,
	.prefs = global_pref_list
};

static pref_val_t global_default_pref_vals[] = {
	{	.pref_val.b = DEFAULT_AUTOSAVE,
	},{	.pref_val.s = DEFAULT_BANNER,
	},{	.pref_val.c = DEFAULT_CMDCHARS,
	},{	.pref_val.i = DEFAULT_COLOR_BLIST_FOCUS,
	},{	.pref_val.i = DEFAULT_COLOR_BLIST_NOFOCUS,
	},{	.pref_val.i = DEFAULT_COLOR_BLIST_SELECTOR,
	},{	.pref_val.s = DEFAULT_FORMAT_ACTION_RECV,
	},{	.pref_val.s = DEFAULT_FORMAT_ACTION_RECV_STATUS,
	},{	.pref_val.s = DEFAULT_FORMAT_ACTION_SEND,
	},{	.pref_val.s = DEFAULT_FORMAT_ACTION_SEND_STATUS,
	},{	.pref_val.s = DEFAULT_FORMAT_BLIST,
	},{	.pref_val.s = DEFAULT_FORMAT_BLIST_GROUP,
	},{	.pref_val.s = DEFAULT_FORMAT_BLIST_IDLE,
	},{	.pref_val.s = DEFAULT_FORMAT_BLIST_WARN,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_CREATE,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_IGNORE,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_INVITE,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_JOIN,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_KICK,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_LEAVE,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_MODE,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_NICK,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_QUIT,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_RECV,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_RECV_ACTION,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_RECV_NOTICE,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_SEND,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_SEND_ACTION,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_SEND_NOTICE,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_TOPIC,
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_UNIGNORE,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_CANCEL_LOCAL,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_CANCEL_REMOTE,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_LOST,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_RECV_ACCEPT,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_RECV_ASK,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_RECV_COMPLETE,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_RECV_RESUME,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_SEND_ACCEPT,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_SEND_ASK,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_SEND_COMPLETE,
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_SEND_RESUME,
	},{	.pref_val.s = DEFAULT_FORMAT_MSG_RECV,
	},{	.pref_val.s = DEFAULT_FORMAT_MSG_RECV_AUTO,
	},{	.pref_val.s = DEFAULT_FORMAT_MSG_RECV_STATUS,
	},{	.pref_val.s = DEFAULT_FORMAT_MSG_SEND,
	},{	.pref_val.s = DEFAULT_FORMAT_MSG_SEND_AUTO,
	},{	.pref_val.s = DEFAULT_FORMAT_MSG_SEND_STATUS,
	},{	.pref_val.s = DEFAULT_FORMAT_NOTICE_RECV,
	},{	.pref_val.s = DEFAULT_FORMAT_NOTICE_RECV_STATUS,
	},{	.pref_val.s = DEFAULT_FORMAT_NOTICE_SEND,
	},{	.pref_val.s = DEFAULT_FORMAT_NOTICE_SEND_STATUS,
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS,
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_ACTIVITY,
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_CHAT,
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_HELD,
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_IDLE,
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_TIMESTAMP,
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_TYPING,
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_WARN,
	},{	.pref_val.s = DEFAULT_FORMAT_TIMESTAMP,
	},{	.pref_val.s = DEFAULT_FORMAT_WARN_SEND,
	},{	.pref_val.s = DEFAULT_FORMAT_WARN_SEND_ANON,
	},{	.pref_val.s = DEFAULT_FORMAT_WARN_RECV,
	},{	.pref_val.s = DEFAULT_FORMAT_WARN_RECV_ANON,
	},{	.pref_val.s = DEFAULT_PORK_DIR,
	},{	.pref_val.b = DEFAULT_RECURSIVE_EVENTS,
	},{	.pref_val.s = DEFAULT_TEXT_BLIST_GROUP_EXPANDED,
	},{	.pref_val.s = DEFAULT_TEXT_BLIST_GROUP_COLLAPSED,
	},{	.pref_val.s = DEFAULT_TEXT_BUDDY_ACTIVE,
	},{	.pref_val.s = DEFAULT_TEXT_BUDDY_AWAY,
	},{	.pref_val.s = DEFAULT_TEXT_BUDDY_IDLE,
	},{	.pref_val.s = DEFAULT_TEXT_BUDDY_WIRELESS,
	},{	.pref_val.s = DEFAULT_TEXT_NO_NAME,
	},{	.pref_val.s = DEFAULT_TEXT_NO_ROOM,
	},{	.pref_val.s = DEFAULT_TEXT_TYPING,
	},{	.pref_val.s = DEFAULT_TEXT_TYPING_PAUSED
	}
};

static struct pref_val global_defaults = {
	.set = &global_pref_set,
	.val = global_default_pref_vals
};

void init_global_prefs(struct screen *globals) {
	globals->prefs = &global_defaults;
}
