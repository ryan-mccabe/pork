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
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>
#include <ncurses.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_queue.h>
#include <pork_inet.h>
#include <pork_misc.h>
#include <pork_set.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>

#include <pork_format.h>
#include <pork_irc.h>
#include <pork_irc_set.h>
#include <pork_set_global.h>
#include <pork_irc_format.h>

static int format_irc_args1(char c, char *buf, size_t len, va_list ap) {
	int ret = -1;
	char *arg = va_arg(ap, char *);

	switch (c) {
		case '1':
		case 'M':
		case 'C':
		case 't':
			if (arg != NULL) {
				char *temp = irc_text_filter(arg);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_irc_args2(char c, char *buf, size_t len, va_list ap) {
	int ret = -1;
	char *arg1 = va_arg(ap, char *);
	char *arg2 = va_arg(ap, char *);

	switch (c) {
		case '1':
		case 'C':
		case 'S':
		case 'U':
		case 'N':
			if (arg1 != NULL) {
				char *temp = irc_text_filter(arg1);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '2':
		case 'M':
		case 't':
			if (arg2 != NULL) {
				char *temp = irc_text_filter(arg2);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_irc_args3(char c, char *buf, size_t len, va_list ap) {
	int ret = -1;
	char *arg1 = va_arg(ap, char *);
	char *arg2 = va_arg(ap, char *);
	char *arg3 = va_arg(ap, char *);

	switch (c) {
		case '1':
		case 'U':
		case 'C':
			if (arg1 != NULL) {
				char *temp = irc_text_filter(arg1);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '2':
		case 'H':
			if (arg2 != NULL) {
				char *temp = irc_text_filter(arg2);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '3':
			if (arg3 != NULL) {
				char *temp = irc_text_filter(arg3);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_irc_args4(char c, char *buf, size_t len, va_list ap) {
	int ret = -1;
	char *arg1 = va_arg(ap, char *);
	char *arg2 = va_arg(ap, char *);
	char *arg3 = va_arg(ap, char *);
	char *arg4 = va_arg(ap, char *);

	switch (c) {
		case '1':
			if (arg1 != NULL) {
				char *temp = irc_text_filter(arg1);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '2':
			if (arg2 != NULL) {
				char *temp = irc_text_filter(arg2);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '3':
			if (arg3 != NULL) {
				char *temp = irc_text_filter(arg3);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '4':
			if (arg4 != NULL) {
				char *temp = irc_text_filter(arg4);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_irc_args5(char c, char *buf, size_t len, va_list ap) {
	int ret = -1;
	char *arg1 = va_arg(ap, char *);
	char *arg2 = va_arg(ap, char *);
	char *arg3 = va_arg(ap, char *);
	char *arg4 = va_arg(ap, char *);
	char *arg5 = va_arg(ap, char *);

	switch (c) {
		case 'D':
		case '1':
			if (arg1 != NULL) {
				char *temp = irc_text_filter(arg1);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '2':
		case 'S':
			if (arg2 != NULL) {
				char *temp = irc_text_filter(arg2);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '3':
		case 'H':
			if (arg3 != NULL) {
				char *temp = irc_text_filter(arg3);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '4':
		case 'A':
			if (arg4 != NULL) {
				char *temp = irc_text_filter(arg4);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case '5':
		case 'M':
			if (arg5 != NULL) {
				char *temp = irc_text_filter(arg5);
				if (temp == NULL)
					return (-1);
				ret = xstrncpy(buf, temp, len);
				free(temp);
			}
			break;

		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

int (*const irc_format_handler[])(char, char *, size_t, va_list) = {
	format_irc_args2,			/* format_irc_chat_created, */
	format_irc_args1,			/* format_irc_chat_mode, */
	format_irc_args2,			/* format_irc_chat_topic, */
	format_irc_args3,			/* format_irc_chat_topic_info, */
	format_irc_args5,			/* format_irc_ctcp_reply, */
	format_irc_args4,			/* format_irc_ctcp_reply_ping, */
	format_irc_args4,			/* format_irc_ctcp_request, */
	format_irc_args4,			/* format_irc_killed, */
	format_irc_args2,			/* format_irc_user_mode, */
	format_irc_args2,			/* format_irc_users, */
	format_irc_args1,			/* format_irc_whois_channels, */
	format_irc_args1,			/* format_irc_whois_idle, */
	format_irc_args1,			/* format_irc_whois_ircname, */
	format_irc_args3,			/* format_irc_whois_nick, */
	format_irc_args2,			/* format_irc_whois_operator, */
	format_irc_args2,			/* format_irc_whois_server, */
	format_irc_args1			/* format_irc_whois_signon */
};
