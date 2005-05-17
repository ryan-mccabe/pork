/*
** pork_set.h - /SET command implementation.
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_SET_H
#define __PORK_SET_H

#define opt_set_format opt_set_str

enum {
	OPT_INT 		= 0,
	OPT_CHAR 		= 1,
	OPT_STR 		= 2,
	OPT_FORMAT 		= 2,
	OPT_BOOL 		= 3,
	OPT_COLOR 		= 4
};

/*
** Global options.
*/

enum {
	OPT_ACTIVITY_TYPES = 0,
	OPT_AUTO_RECONNECT,
	OPT_AUTO_REJOIN,
	OPT_AUTOSEND_AWAY,
	OPT_BANNER,
	OPT_BEEP,
	OPT_BEEP_MAX,
	OPT_BEEP_ON_OUTPUT,
	OPT_CMDCHARS,
	OPT_COLOR_BLIST_FOCUS,
	OPT_COLOR_BLIST_NOFOCUS,
	OPT_COLOR_BLIST_SELECTOR,
	OPT_CONNECT_TIMEOUT,
	OPT_DOWNLOAD_DIR,
	OPT_DUMP_MSGS_TO_STATUS,
	OPT_FORMAT_ACTION_RECV,
	OPT_FORMAT_ACTION_RECV_STATUS,
	OPT_FORMAT_ACTION_SEND,
	OPT_FORMAT_ACTION_SEND_STATUS,
	OPT_FORMAT_BLIST,
	OPT_FORMAT_BLIST_GROUP,
	OPT_FORMAT_BLIST_IDLE,
	OPT_FORMAT_BLIST_WARN,
	OPT_FORMAT_CHAT_CREATE,
	OPT_FORMAT_CHAT_IGNORE,
	OPT_FORMAT_CHAT_INVITE,
	OPT_FORMAT_CHAT_JOIN,
	OPT_FORMAT_CHAT_KICK,
	OPT_FORMAT_CHAT_LEAVE,
	OPT_FORMAT_CHAT_MODE,
	OPT_FORMAT_CHAT_NICK,
	OPT_FORMAT_CHAT_QUIT,
	OPT_FORMAT_CHAT_RECV,
	OPT_FORMAT_CHAT_RECV_ACTION,
	OPT_FORMAT_CHAT_RECV_NOTICE,
	OPT_FORMAT_CHAT_SEND,
	OPT_FORMAT_CHAT_SEND_ACTION,
	OPT_FORMAT_CHAT_SEND_NOTICE,
	OPT_FORMAT_CHAT_TOPIC,
	OPT_FORMAT_CHAT_UNIGNORE,
	OPT_FORMAT_FILE_CANCEL_LOCAL,
	OPT_FORMAT_FILE_CANCEL_REMOTE,
	OPT_FORMAT_FILE_LOST,
	OPT_FORMAT_FILE_RECV_ACCEPT,
	OPT_FORMAT_FILE_RECV_ASK,
	OPT_FORMAT_FILE_RECV_COMPLETE,
	OPT_FORMAT_FILE_RECV_RESUME,
	OPT_FORMAT_FILE_SEND_ACCEPT,
	OPT_FORMAT_FILE_SEND_ASK,
	OPT_FORMAT_FILE_SEND_COMPLETE,
	OPT_FORMAT_FILE_SEND_RESUME,
	OPT_FORMAT_IM_RECV,
	OPT_FORMAT_IM_RECV_AUTO,
	OPT_FORMAT_IM_RECV_STATUS,
	OPT_FORMAT_IM_SEND,
	OPT_FORMAT_IM_SEND_AUTO,
	OPT_FORMAT_IM_SEND_STATUS,
	OPT_FORMAT_NOTICE_RECV,
	OPT_FORMAT_NOTICE_RECV_STATUS,
	OPT_FORMAT_NOTICE_SEND,
	OPT_FORMAT_NOTICE_SEND_STATUS,
	OPT_FORMAT_STATUS,
	OPT_FORMAT_STATUS_ACTIVITY,
	OPT_FORMAT_STATUS_CHAT,
	OPT_FORMAT_STATUS_HELD,
	OPT_FORMAT_STATUS_IDLE,
	OPT_FORMAT_STATUS_TIMESTAMP,
	OPT_FORMAT_STATUS_TYPING,
	OPT_FORMAT_STATUS_WARN,
	OPT_FORMAT_TIMESTAMP,
	OPT_FORMAT_WARN,
	OPT_FORMAT_WHOIS_AWAY,
	OPT_FORMAT_WHOIS_IDLE,
	OPT_FORMAT_WHOIS_MEMBER,
	OPT_FORMAT_WHOIS_NAME,
	OPT_FORMAT_WHOIS_SIGNON,
	OPT_FORMAT_WHOIS_USERINFO,
	OPT_FORMAT_WHOIS_WARNLEVEL,
	OPT_HISTORY_LEN,
	OPT_IDLE_AFTER,
	OPT_LOG,
	OPT_LOG_TYPES,
	OPT_LOGIN_ON_STARTUP,
	OPT_OUTGOING_MSG_FONT,
	OPT_OUTGOING_MSG_FONT_BGCOLOR,
	OPT_OUTGOING_MSG_FONT_FGCOLOR,
	OPT_OUTGOING_MSG_FONT_SIZE,
	OPT_PORK_DIR,
	OPT_PRIVATE_INPUT,
	OPT_PROMPT,
	OPT_RECONNECT_INTERVAL,
	OPT_RECONNECT_MAX_INTERVAL,
	OPT_RECONNECT_TRIES,
	OPT_RECURSIVE_EVENTS,
	OPT_REPORT_IDLE,
	OPT_SAVE_PASSWD,
	OPT_SCROLL_ON_INPUT,
	OPT_SCROLL_ON_OUTPUT,
	OPT_SCROLLBUF_LEN,
	OPT_SEND_REMOVES_AWAY,
	OPT_SHOW_BLIST,
	OPT_SHOW_BUDDY_AWAY,
	OPT_SHOW_BUDDY_IDLE,
	OPT_SHOW_BUDDY_SIGNOFF,
	OPT_TEXT_BLIST_GROUP_COLLAPSED,
	OPT_TEXT_BLIST_GROUP_EXPANDED,
	OPT_TEXT_BUDDY_ACTIVE,
	OPT_TEXT_BUDDY_AWAY,
	OPT_TEXT_BUDDY_IDLE,
	OPT_TEXT_NO_NAME,
	OPT_TEXT_NO_ROOM,
	OPT_TEXT_TYPING,
	OPT_TEXT_TYPING_PAUSED,
	OPT_TEXT_WARN_ANONYMOUS,
	OPT_TRANSFER_PORT_MAX,
	OPT_TRANSFER_PORT_MIN,
	OPT_WORDWRAP,
	OPT_WORDWRAP_CHAR,
	OPT_NUM_OPTS
};

#define OPT_FORMAT_OFFSET OPT_FORMAT_ACTION_RECV

/*
** Per-window options.
*/

struct imwindow;

typedef union {
	u_int32_t i;
	u_int32_t b;
	char c;
	char *s;
} pref_val_t;

struct global_pref {
	char *name;
	u_int32_t type:31;
	u_int32_t dynamic:1;
	int (*set)(u_int32_t, char *);
	void (*updated)(void);
	pref_val_t val;
};

extern struct global_pref global_pref[OPT_NUM_OPTS];

int opt_set_bool(u_int32_t opt, char *args);
int opt_set_char(u_int32_t opt, char *args);
int opt_set_int(u_int32_t opt, char *args);
int opt_set_str(u_int32_t opt, char *args);
int opt_set_color(u_int32_t opt, char *args);

#define SET_STR(x)	{ .s = (x) }
#define SET_INT(x)	{ .i = (x) }
#define SET_CHAR(x)	{ .c = (x) }
#define SET_BOOL(x)	{ .b = (x) }

void opt_destroy(void);
void opt_print_var(int var, const char *text);
void opt_print(void);
void opt_write(FILE *fp);
int opt_set(u_int32_t opt, char *args);
int opt_tristate(char *args);
int opt_find(const char *name);
int opt_get_val(const char *opt_name, char *buf, size_t len);

/*
** These used to be inline functions until I discovered
** how positively stupid GCC is. GCC refused to inline
** the functions regardless of which one of the 9 or 10
** possible ways to tell it "inline this" I used.
*/

#define opt_get_int(opt) (global_pref[(opt)].val.i)
#define opt_get_color(opt) (global_pref[(opt)].val.i)
#define opt_get_bool(opt) (global_pref[(opt)].val.b)
#define opt_get_char(opt) (global_pref[(opt)].val.c)
#define opt_get_str(opt) (global_pref[(opt)].val.s)

#include "pork_set_defaults.h"

#endif
