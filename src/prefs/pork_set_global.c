/*
** pork_set_global.c - global /SET command implementation.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
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
#include <pork_imwindow.h>
#include <pork_acct.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

/*
** Name:		The name of the option
** Type:		The type of the option (boolean, string, int, char, color)
** Set func:	The function that checks and sets the requested value of the opt
** Change func:	The function called when the value of the option changes
*/

static struct pork_pref global_pref_list[] = {
	{	"BANNER",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{	"CMDCHARS",
		OPT_TYPE_CHAR,
		opt_set_char,
		NULL,
	},{	"COLOR_BLIST_FOCUS",
		OPT_TYPE_COLOR,
		opt_set_color,
		pork_acct_update_blist_color,
	},{	"COLOR_BLIST_NOFOCUS",
		OPT_TYPE_COLOR,
		opt_set_color,
		pork_acct_update_blist_color,
	},{	"COLOR_BLIST_SELECTOR",
		OPT_TYPE_COLOR,
		opt_set_color,
		pork_acct_update_blist_color,
	},{	"FORMAT_ACTION_RECV",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_ACTION_RECV_STATUS",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_ACTION_SEND",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_ACTION_SEND_STATUS",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_BLIST",
		OPT_TYPE_FORMAT,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"FORMAT_BLIST_GROUP",
		OPT_TYPE_FORMAT,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"FORMAT_BLIST_IDLE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"FORMAT_BLIST_WARN",
		OPT_TYPE_FORMAT,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"FORMAT_CHAT_CREATE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_IGNORE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_INVITE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_JOIN",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_KICK",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_LEAVE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_MODE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_NICK",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_QUIT",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_RECV",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_RECV_ACTION",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_RECV_NOTICE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_SEND",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_SEND_ACTION",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_SEND_NOTICE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_TOPIC",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_CHAT_UNIGNORE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_CANCEL_LOCAL",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_CANCEL_REMOTE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_LOST",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_RECV_ACCEPT",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_RECV_ASK",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_RECV_COMPLETE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_RECV_RESUME",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_SEND_ACCEPT",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_SEND_ASK",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_SEND_COMPLETE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_FILE_SEND_RESUME",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_IM_RECV",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_IM_RECV_AUTO",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_IM_RECV_STATUS",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_IM_SEND",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_IM_SEND_AUTO",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_IM_SEND_STATUS",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_NOTICE_RECV",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_NOTICE_RECV_STATUS",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_NOTICE_SEND",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_NOTICE_SEND_STATUS",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_STATUS",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_STATUS_ACTIVITY",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_STATUS_CHAT",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_STATUS_HELD",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_STATUS_IDLE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_STATUS_TIMESTAMP",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_STATUS_TYPING",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_STATUS_WARN",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_TIMESTAMP",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_WARN",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_WHOIS_AWAY",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_WHOIS_IDLE",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_WHOIS_MEMBER",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_WHOIS_NAME",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_WHOIS_SIGNON",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_WHOIS_USERINFO",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"FORMAT_WHOIS_WARNLEVEL",
		OPT_TYPE_FORMAT,
		opt_set_format,
		NULL,
	},{	"OUTGOING_MSG_FONT",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{	"OUTGOING_MSG_FONT_BGCOLOR",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{	"OUTGOING_MSG_FONT_FGCOLOR",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{	"OUTGOING_MSG_FONT_SIZE",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{ "RECURSIVE_EVENTS",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL,
	},{	"TEXT_BLIST_GROUP_COLLAPSED",
		OPT_TYPE_STR,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"TEXT_BLIST_GROUP_EXPANDED",
		OPT_TYPE_STR,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"TEXT_BUDDY_ACTIVE",
		OPT_TYPE_STR,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"TEXT_BUDDY_AWAY",
		OPT_TYPE_STR,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"TEXT_BUDDY_IDLE",
		OPT_TYPE_STR,
		opt_set_format,
		pork_acct_update_blist_format,
	},{	"TEXT_NO_NAME",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{	"TEXT_NO_ROOM",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{	"TEXT_TYPING",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{	"TEXT_TYPING_PAUSED",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	},{	"TEXT_WARN_ANONYMOUS",
		OPT_TYPE_STR,
		opt_set_str,
		NULL,
	}
};

static const struct pref_set global_pref_set = {
	.name = NULL,
	.num_opts = GLOBAL_NUM_OPTS,
	.prefs = global_pref_list
};

static pref_val_t global_default_pref_vals[] = {
	{	.pref_val.s = DEFAULT_BANNER,
		.dynamic = 0
	},{	.pref_val.c = DEFAULT_CMDCHARS,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_COLOR_BLIST_FOCUS,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_COLOR_BLIST_NOFOCUS,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_COLOR_BLIST_SELECTOR,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_ACTION_RECV,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_ACTION_RECV_STATUS,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_ACTION_SEND,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_ACTION_SEND_STATUS,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_BLIST,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_BLIST_GROUP,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_BLIST_IDLE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_BLIST_WARN,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_CREATE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_IGNORE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_INVITE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_JOIN,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_KICK,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_LEAVE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_MODE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_NICK,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_QUIT,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_RECV,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_RECV_ACTION,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_RECV_NOTICE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_SEND,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_SEND_ACTION,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_SEND_NOTICE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_TOPIC,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_CHAT_UNIGNORE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_CANCEL_LOCAL,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_CANCEL_REMOTE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_LOST,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_RECV_ACCEPT,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_RECV_ASK,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_RECV_COMPLETE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_RECV_RESUME,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_SEND_ACCEPT,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_SEND_ASK,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_SEND_COMPLETE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_FILE_SEND_RESUME,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_IM_RECV,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_IM_RECV_AUTO,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_IM_RECV_STATUS,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_IM_SEND,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_IM_SEND_AUTO,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_IM_SEND_STATUS,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_NOTICE_RECV,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_NOTICE_RECV_STATUS,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_NOTICE_SEND,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_NOTICE_SEND_STATUS,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_ACTIVITY,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_CHAT,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_HELD,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_IDLE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_TIMESTAMP,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_TYPING,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_STATUS_WARN,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_TIMESTAMP,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_WARN,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_WHOIS_AWAY,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_WHOIS_IDLE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_WHOIS_MEMBER,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_WHOIS_NAME,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_WHOIS_SIGNON,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_WHOIS_USERINFO,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_FORMAT_WHOIS_WARNLEVEL,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_OUTGOING_MSG_FONT,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_OUTGOING_MSG_FONT_BGCOLOR,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_OUTGOING_MSG_FONT_FGCOLOR,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_OUTGOING_MSG_FONT_SIZE,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_RECURSIVE_EVENTS,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_BLIST_GROUP_EXPANDED,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_BLIST_GROUP_COLLAPSED,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_BUDDY_ACTIVE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_BUDDY_AWAY,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_BUDDY_IDLE,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_NO_NAME,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_NO_ROOM,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_TYPING,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_TYPING_PAUSED,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_TEXT_WARN_ANONYMOUS,
		.dynamic = 0
	}
};

static struct pref_val global_defaults = {
	.set = &global_pref_set,
	.val = global_default_pref_vals
};

void init_global_prefs(struct screen *screen) {
	screen->global_prefs = &global_defaults;
}
