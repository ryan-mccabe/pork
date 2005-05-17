/*
** pork_set.c - /SET command implementation.
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
#include <pork_imwindow.h>
#include <pork_acct.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

extern struct screen screen;

static void opt_changed_prompt(void);
static void scrollbuf_len_update(void);

/*
** Name:		The name of the option
** Type:		The type of the option (boolean, string, int, char, color)
** Dynamic:		When the option changes, does the current value need to
**				be freed before the new one is set (i.e. was it
**				dynamically allocated?).
** Set func:	The function used to set the value of the option.
** Change func:	The function to be called when the value of the option changes.
** Default:		The default value of the option.
*/

struct global_pref global_pref[] = {
	{	"ACTIVITY_TYPES",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_ACTIVITY_TYPES)
	},{	"AUTO_RECONNECT",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_AUTO_RECONNECT)
	},{	"AUTO_REJOIN",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_AUTO_REJOIN)
	},{	"AUTOSEND_AWAY",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_AUTOSEND_AWAY)
	},{	"BANNER",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_BANNER)
	},{	"BEEP",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_BEEP)
	},{	"BEEP_MAX",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_BEEP_MAX)
	},{	"BEEP_ON_OUTPUT",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_BEEP_ON_OUTPUT)
	},{	"CMDCHARS",
		OPT_CHAR,
		0,
		opt_set_char,
		NULL,
		SET_CHAR(DEFAULT_CMDCHARS),
	},{	"COLOR_BLIST_FOCUS",
		OPT_COLOR,
		0,
		opt_set_color,
		pork_acct_update_blist_color,
		SET_INT(DEFAULT_COLOR_BLIST_FOCUS),
	},{	"COLOR_BLIST_NOFOCUS",
		OPT_COLOR,
		0,
		opt_set_color,
		pork_acct_update_blist_color,
		SET_INT(DEFAULT_COLOR_BLIST_NOFOCUS),
	},{	"COLOR_BLIST_SELECTOR",
		OPT_COLOR,
		0,
		opt_set_color,
		pork_acct_update_blist_color,
		SET_INT(DEFAULT_COLOR_BLIST_SELECTOR),
	},{	"CONNECT_TIMEOUT",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_CONNECT_TIMEOUT)
	},{	"DOWNLOAD_DIR",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_DOWNLOAD_DIR),
	},{	"DUMP_MSGS_TO_STATUS",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_DUMP_MSGS_TO_STATUS),
	},{	"FORMAT_ACTION_RECV",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_ACTION_RECV),
	},{	"FORMAT_ACTION_RECV_STATUS",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_ACTION_RECV_STATUS),
	},{	"FORMAT_ACTION_SEND",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_ACTION_SEND),
	},{	"FORMAT_ACTION_SEND_STATUS",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_ACTION_SEND_STATUS),
	},{	"FORMAT_BLIST",
		OPT_FORMAT,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_FORMAT_BLIST),
	},{	"FORMAT_BLIST_GROUP",
		OPT_FORMAT,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_FORMAT_BLIST_GROUP),
	},{	"FORMAT_BLIST_IDLE",
		OPT_FORMAT,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_FORMAT_BLIST_IDLE),
	},{	"FORMAT_BLIST_WARN",
		OPT_FORMAT,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_FORMAT_BLIST_WARN),
	},{	"FORMAT_CHAT_CREATE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_CREATE),
	},{	"FORMAT_CHAT_IGNORE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_IGNORE),
	},{	"FORMAT_CHAT_INVITE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_INVITE),
	},{	"FORMAT_CHAT_JOIN",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_JOIN),
	},{	"FORMAT_CHAT_KICK",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_KICK),
	},{	"FORMAT_CHAT_LEAVE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_LEAVE),
	},{	"FORMAT_CHAT_MODE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_MODE),
	},{	"FORMAT_CHAT_NICK",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_NICK),
	},{	"FORMAT_CHAT_QUIT",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_QUIT),
	},{	"FORMAT_CHAT_RECV",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_RECV),
	},{	"FORMAT_CHAT_RECV_ACTION",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_RECV_ACTION),
	},{	"FORMAT_CHAT_RECV_NOTICE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_RECV_NOTICE),
	},{	"FORMAT_CHAT_SEND",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_SEND),
	},{	"FORMAT_CHAT_SEND_ACTION",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_SEND_ACTION),
	},{	"FORMAT_CHAT_SEND_NOTICE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_SEND_NOTICE),
	},{	"FORMAT_CHAT_TOPIC",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_TOPIC),
	},{	"FORMAT_CHAT_UNIGNORE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_CHAT_UNIGNORE),
	},{	"FORMAT_FILE_CANCEL_LOCAL",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_CANCEL_LOCAL),
	},{	"FORMAT_FILE_CANCEL_REMOTE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_CANCEL_REMOTE),
	},{	"FORMAT_FILE_LOST",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_LOST),
	},{	"FORMAT_FILE_RECV_ACCEPT",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_RECV_ACCEPT),
	},{	"FORMAT_FILE_RECV_ASK",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_RECV_ASK),
	},{	"FORMAT_FILE_RECV_COMPLETE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_RECV_COMPLETE),
	},{	"FORMAT_FILE_RECV_RESUME",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_RECV_RESUME),
	},{	"FORMAT_FILE_SEND_ACCEPT",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_SEND_ACCEPT),
	},{	"FORMAT_FILE_SEND_ASK",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_SEND_ASK),
	},{	"FORMAT_FILE_SEND_COMPLETE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_SEND_COMPLETE),
	},{	"FORMAT_FILE_SEND_RESUME",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_FILE_SEND_RESUME),
	},{	"FORMAT_IM_RECV",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_IM_RECV),
	},{	"FORMAT_IM_RECV_AUTO",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_IM_RECV_AUTO),
	},{	"FORMAT_IM_RECV_STATUS",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_IM_RECV_STATUS),
	},{	"FORMAT_IM_SEND",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_IM_SEND),
	},{	"FORMAT_IM_SEND_AUTO",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_IM_SEND_AUTO),
	},{	"FORMAT_IM_SEND_STATUS",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_IM_SEND_STATUS),
	},{	"FORMAT_NOTICE_RECV",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_NOTICE_RECV),
	},{	"FORMAT_NOTICE_RECV_STATUS",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_NOTICE_RECV_STATUS),
	},{	"FORMAT_NOTICE_SEND",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_NOTICE_SEND),
	},{	"FORMAT_NOTICE_SEND_STATUS",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_NOTICE_SEND_STATUS),
	},{	"FORMAT_STATUS",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_STATUS),
	},{	"FORMAT_STATUS_ACTIVITY",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_STATUS_ACTIVITY),
	},{	"FORMAT_STATUS_CHAT",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_STATUS_CHAT),
	},{	"FORMAT_STATUS_HELD",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_STATUS_HELD),
	},{	"FORMAT_STATUS_IDLE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_STATUS_IDLE),
	},{	"FORMAT_STATUS_TIMESTAMP",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_STATUS_TIMESTAMP),
	},{	"FORMAT_STATUS_TYPING",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_STATUS_TYPING),
	},{	"FORMAT_STATUS_WARN",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_STATUS_WARN),
	},{	"FORMAT_TIMESTAMP",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_TIMESTAMP),
	},{	"FORMAT_WARN",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_WARN),
	},{	"FORMAT_WHOIS_AWAY",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_WHOIS_AWAY),
	},{ "FORMAT_WHOIS_IDLE",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_WHOIS_IDLE),
	},{ "FORMAT_WHOIS_MEMBER",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_WHOIS_MEMBER),
	},{ "FORMAT_WHOIS_NAME",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_WHOIS_NAME),
	},{ "FORMAT_WHOIS_SIGNON",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_WHOIS_SIGNON),
	},{	"FORMAT_WHOIS_USERINFO",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_WHOIS_USERINFO),
	},{	"FORMAT_WHOIS_WARNLEVEL",
		OPT_FORMAT,
		0,
		opt_set_format,
		NULL,
		SET_STR(DEFAULT_FORMAT_WHOIS_WARNLEVEL),
	},{	"HISTORY_LEN",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_HISTORY_LEN),
	},{	"IDLE_AFTER",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_IDLE_AFTER),
	},{	"LOG",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_LOG),
	},{	"LOG_TYPES",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_LOG_TYPES)
	},{	"LOGIN_ON_STARTUP",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_LOGIN_ON_STARTUP),
	},{ "OUTGOING_MSG_FONT",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_OUTGOING_MSG_FONT),
	},{ "OUTGOING_MSG_FONT_BGCOLOR",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_OUTGOING_MSG_FONT_BGCOLOR),
	},{ "OUTGOING_MSG_FONT_FGCOLOR",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_OUTGOING_MSG_FONT_FGCOLOR),
	},{ "OUTGOING_MSG_FONT_SIZE",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_OUTGOING_MSG_FONT_SIZE),
	},{	"PORK_DIR",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_PORK_DIR),
	},{	"PRIVATE_INPUT",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_PRIVATE_INPUT),
	},{	"PROMPT",
		OPT_FORMAT,
		0,
		opt_set_format,
		opt_changed_prompt,
		SET_STR(DEFAULT_PROMPT),
	},{	"RECONNECT_INTERVAL",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_RECONNECT_INTERVAL),
	},{	"RECONNECT_MAX_INTERVAL",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_RECONNECT_MAX_INTERVAL),
	},{	"RECONNECT_TRIES",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_RECONNECT_TRIES),
	},{	"RECURSIVE_EVENTS",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_RECURSIVE_EVENTS),
	},{	"REPORT_IDLE",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_REPORT_IDLE),
	},{	"SAVE_PASSWD",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_SAVE_PASSWD),
	},{	"SCROLL_ON_INPUT",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_SCROLL_ON_INPUT),
	},{	"SCROLL_ON_OUTPUT",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_SCROLL_ON_OUTPUT),
	},{	"SCROLLBUF_LEN",
		OPT_INT,
		0,
		opt_set_int,
		scrollbuf_len_update,
		SET_INT(DEFAULT_SCROLLBUF_LEN),
	},{	"SEND_REMOVES_AWAY",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_SEND_REMOVES_AWAY),
	},{	"SHOW_BLIST",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_SHOW_BLIST),
	},{	"SHOW_BUDDY_AWAY",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_SHOW_BUDDY_AWAY),
	},{	"SHOW_BUDDY_IDLE",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_SHOW_BUDDY_IDLE),
	},{	"SHOW_BUDDY_SIGNOFF",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_SHOW_BUDDY_SIGNOFF),
	},{	"TEXT_BLIST_GROUP_COLLAPSED",
		OPT_STR,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_TEXT_BLIST_GROUP_COLLAPSED),
	},{	"TEXT_BLIST_GROUP_EXPANDED",
		OPT_STR,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_TEXT_BLIST_GROUP_EXPANDED),
	},{	"TEXT_BUDDY_ACTIVE",
		OPT_STR,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_TEXT_BUDDY_ACTIVE),
	},{	"TEXT_BUDDY_AWAY",
		OPT_STR,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_TEXT_BUDDY_AWAY),
	},{	"TEXT_BUDDY_IDLE",
		OPT_STR,
		0,
		opt_set_format,
		pork_acct_update_blist_format,
		SET_STR(DEFAULT_TEXT_BUDDY_IDLE),
	},{	"TEXT_NO_NAME",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_TEXT_NO_NAME),
	},{	"TEXT_NO_ROOM",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_TEXT_NO_ROOM),
	},{	"TEXT_TYPING",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_TEXT_TYPING),
	},{	"TEXT_TYPING_PAUSED",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_TEXT_TYPING_PAUSED),
	},{	"TEXT_WARN_ANONYMOUS",
		OPT_STR,
		0,
		opt_set_str,
		NULL,
		SET_STR(DEFAULT_TEXT_WARN_ANONYMOUS),
	},{	"TRANSFER_PORT_MAX",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_TRANSFER_PORT_MAX),
	},{	"TRANSFER_PORT_MIN",
		OPT_INT,
		0,
		opt_set_int,
		NULL,
		SET_INT(DEFAULT_TRANSFER_PORT_MIN),
	},{	"WORDWRAP",
		OPT_BOOL,
		0,
		opt_set_bool,
		NULL,
		SET_BOOL(DEFAULT_WORDWRAP),
	},{	"WORDWRAP_CHAR",
		OPT_CHAR,
		0,
		opt_set_char,
		NULL,
		SET_CHAR(DEFAULT_WORDWRAP_CHAR),
	}
};

int opt_get_val(const char *opt_name, char *buf, size_t len) {
	int i = opt_find(opt_name);

	if (i == -1)
		return (-1);

	switch (global_pref[i].type) {
		case OPT_BOOL:
			snprintf(buf, len, "%d", global_pref[i].val.b);
			break;

		case OPT_STR:
			if (global_pref[i].val.s == NULL)
				return (-1);
			xstrncpy(buf, global_pref[i].val.s, len);
			break;

		case OPT_INT:
			snprintf(buf, len, "%d", global_pref[i].val.i);
			break;

		case OPT_CHAR:
			snprintf(buf, len, "%c", global_pref[i].val.c);
			break;

		case OPT_COLOR:
			color_get_str(global_pref[i].val.i, buf, len);
			break;
	}

	return (0);
}

void opt_print_var(int i, const char *text) {
	switch (global_pref[i].type) {
		case OPT_BOOL:
			screen_nocolor_msg("%s %s %s", global_pref[i].name,
				text, (global_pref[i].val.b ? "TRUE" : "FALSE"));
			break;

		case OPT_STR:
			if (global_pref[i].val.s != NULL &&
				global_pref[i].val.s[0] != '\0')
			{
				screen_nocolor_msg("%s %s \"%s\"", global_pref[i].name,
					text, global_pref[i].val.s);
			} else
				screen_nocolor_msg("%s is <UNSET>", global_pref[i].name);

			break;

		case OPT_INT:
			screen_nocolor_msg("%s %s %d", global_pref[i].name,
				text, global_pref[i].val.i);
			break;

		case OPT_CHAR:
			if (isprint(global_pref[i].val.c)) {
				screen_nocolor_msg("%s %s '%c'", global_pref[i].name,
					text, global_pref[i].val.c);
			} else {
				screen_nocolor_msg("%s %s 0x%x", global_pref[i].name,
					text, global_pref[i].val.c);
			}
			break;

		case OPT_COLOR: {
			char buf[128];

			color_get_str(global_pref[i].val.i, buf, sizeof(buf));
			screen_nocolor_msg("%s %s %s", global_pref[i].name, text, buf);
			break;
		}
	}
}

static void scrollbuf_len_update(void) {
	u_int32_t scrollbuf_len;

	scrollbuf_len = opt_get_int(OPT_SCROLLBUF_LEN);
	if (scrollbuf_len < cur_window()->swindow.rows)
		global_pref[OPT_SCROLLBUF_LEN].val.i = cur_window()->swindow.rows;
}

/*
** Print the values of all the global variables.
*/

void opt_print(void) {
	size_t i;

	for (i = 0 ; i < array_elem(global_pref) ; i++)
		opt_print_var(i, "is set to");
}

void opt_write(FILE *fp) {
	size_t i;

	for (i = 0 ; i < array_elem(global_pref) ; i++) {
		switch (global_pref[i].type) {
			case OPT_BOOL:
				fprintf(fp, "set %s %s\n", global_pref[i].name,
					(global_pref[i].val.b ? "TRUE" : "FALSE"));
				break;

			case OPT_STR:
				if (global_pref[i].val.s != NULL &&
					global_pref[i].val.s[0] != '\0')
				{
					fprintf(fp, "set %s %s\n", global_pref[i].name,
						global_pref[i].val.s);
				}
				break;

			case OPT_INT:
				fprintf(fp, "set %s %d\n", global_pref[i].name,
					global_pref[i].val.i);
				break;

			case OPT_CHAR:
				if (isprint(global_pref[i].val.c)) {
					fprintf(fp, "set %s %c\n", global_pref[i].name,
						global_pref[i].val.c);
				} else {
					fprintf(fp, "set %s 0x%x\n", global_pref[i].name,
						global_pref[i].val.c);
				}
				break;

			case OPT_COLOR: {
				char buf[128];

				color_get_str(global_pref[i].val.i, buf, sizeof(buf));
				fprintf(fp, "set %s %s\n", global_pref[i].name, buf);
				break;
			}
		}
	}
}

static int opt_compare(const void *l, const void *r) {
	const char *str = l;
	const struct global_pref *gvar = r;

	return (strcasecmp(str, gvar->name));
}

/*
** Find the position of the global option named "name"
** in the global option table.
*/

int opt_find(const char *name) {
	struct global_pref *gvar;
	u_int32_t offset;

	gvar = bsearch(name, global_pref, array_elem(global_pref),
				sizeof(struct global_pref), opt_compare);

	if (gvar == NULL)
		return (-1);

	offset = (long) gvar - (long) &global_pref[0];
	return (offset / sizeof(struct global_pref));
}

static void opt_changed_prompt(void) {
	input_set_prompt(&screen.input, opt_get_str(OPT_PROMPT));
}

/*
** The values a boolean variable can accept are TRUE, FALSE and TOGGLE.
** TRUE is 1, FALSE is 0. TOGGLE flips the current value of the variable.
*/

int opt_tristate(char *args) {
	if (args == NULL)
		return (-1);

	if (!strcasecmp(args, "ON") ||
		!strcasecmp(args, "TRUE") ||
		!strcasecmp(args, "1"))
	{
		return (1);
	}

	if (!strcasecmp(args, "OFF") ||
		!strcasecmp(args, "FALSE") ||
		!strcasecmp(args, "0"))
	{
		return (0);
	}

	if (!strcasecmp(args, "TOGGLE"))
		return (2);

	return (-1);
}

int opt_set_bool(u_int32_t opt, char *args) {
	int val = opt_tristate(args);

	if (val == -1)
		return (-1);

	if (val != 2)
		global_pref[opt].val.b = val;
	else
		global_pref[opt].val.b = !global_pref[opt].val.b;

	if (global_pref[opt].updated != NULL)
		global_pref[opt].updated();

	return (0);
}

int opt_set_char(u_int32_t opt, char *args) {
	if (args == NULL || *args == '\0')
		return (-1);

	if (!strncasecmp(args, "0x", 2)) {
		u_int32_t temp;

		if (str_to_uint(args, &temp) == -1 || temp > 0xff)
			global_pref[opt].val.c = *args;
		else
			global_pref[opt].val.c = temp;
	} else
		global_pref[opt].val.c = *args;

	if (global_pref[opt].updated != NULL)
		global_pref[opt].updated();

	return (0);
}

int opt_set_int(u_int32_t opt, char *args) {
	u_int32_t num;

	if (args == NULL)
		return (-1);

	if (str_to_uint(args, &num) != 0)
		return (-1);

	global_pref[opt].val.i = num;

	if (global_pref[opt].updated != NULL)
		global_pref[opt].updated();

	return (0);
}

int opt_set_str(u_int32_t opt, char *args) {
	if (global_pref[opt].dynamic == 1)
		free(global_pref[opt].val.s);

	if (args != NULL) {
		global_pref[opt].val.s = xstrdup(args);
		global_pref[opt].dynamic = 1;
	} else {
		global_pref[opt].val.s = NULL;
		global_pref[opt].dynamic = 0;
	}

	if (global_pref[opt].updated != NULL)
		global_pref[opt].updated();

	return (0);
}

int opt_set_color(u_int32_t opt, char *args) {
	attr_t attr = 0;

	if (*args != '%') {
		if (color_parse_code(args, &attr) == -1)
			return (-1);
	} else {
		char buf[32];
		chtype ch[4];

		snprintf(buf, sizeof(buf), "%s ", args);
		if (plaintext_to_cstr(ch, array_elem(ch), buf, NULL) != 1)
			return (-1);

		attr = ch[0] & A_ATTRIBUTES;
	}

	global_pref[opt].val.i = attr;

	if (global_pref[opt].updated != NULL)
		global_pref[opt].updated();

	return (0);
}

int opt_set(u_int32_t opt, char *args) {
	struct global_pref *var = &global_pref[opt];

	return (var->set(opt, args));
}

void opt_destroy(void) {
	size_t i;

	for (i = 0 ; i < array_elem(global_pref) ; i++) {
		if (global_pref[i].dynamic)
			free(global_pref[i].val.s);
	}
}
