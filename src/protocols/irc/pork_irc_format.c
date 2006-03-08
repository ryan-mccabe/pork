/*
** pork_irc_format.c
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/types.h>
#include <ncurses.h>

#include <pork.h>
#include <pork_list.h>
#include <pork_util.h>
#include <pork_inet.h>
#include <pork_misc.h>
#include <pork_set.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>

#include <pork_format.h>
#include <pork_irc_set.h>
#include <pork_set_global.h>
#include <pork_irc_format.h>

static int format_irc_chat_info(char c, char *buf, size_t len, va_list ap) {
	char *chat_name = va_arg(ap, char *);
	char *msg = va_arg(ap, char *);
	int ret;

	switch (c) {
        case 'T':
            ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
            break;
	}
		
	return (-1);
}

static int format_irc_fixme(char c, char *buf, size_t len, va_list ap) {
	return (-1);
}

static int (*const irc_format_handler[])(char, char *, size_t, va_list) = {
	format_irc_fixme,			/* format_irc_chat_created, */
	format_irc_fixme,			/* format_irc_chat_info, */
	format_irc_fixme,			/* format_irc_chat_mode, */
	format_irc_fixme,			/* format_irc_chat_topic, */
	format_irc_fixme,			/* format_irc_chat_topic_info, */
	format_irc_fixme,			/* format_irc_ctcp_reply, */
	format_irc_fixme,			/* format_irc_ctcp_reply_ping, */
	format_irc_fixme,			/* format_irc_ctcp_request, */
	format_irc_fixme,			/* format_irc_killed, */
	format_irc_fixme,			/* format_irc_user_mode, */
	format_irc_fixme,			/* format_irc_users, */
	format_irc_fixme,			/* format_irc_whois_away, */
	format_irc_fixme,			/* format_irc_whois_channels, */
	format_irc_fixme,			/* format_irc_whois_idle, */
	format_irc_fixme,			/* format_irc_whois_ircname, */
	format_irc_fixme,			/* format_irc_whois_nick, */
	format_irc_fixme,			/* format_irc_whois_operator, */
	format_irc_fixme,			/* format_irc_whois_server, */
	format_irc_fixme			/* format_irc_whois_signon */
};

int irc_fill_format_str(struct pork_acct *acct, int type, char *buf, size_t len, ...) {
	int ret;

	ret = fill_format_string(type, buf, len, acct->proto_prefs,
			irc_format_handler[IRC_OPT_FORMAT_OFFSET - type]);

	return (ret);
}
