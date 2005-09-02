/*
** pork_irc_format.c
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
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
#include <pork_misc.h>
#include <pork_set.h>

#include <pork_format.h>

static int format_irc_chat_info(char c, char *buf, size_t len, va_list ap) {
	return (-1);
}

static int format_irc_ctcp(char c, char *buf, size_t len, va_list ap) {
	return (-1);
}

static int format_irc_umode(char c, char *buf, size_t len, va_list ap) {
	return (-1);
}

static int format_irc_users(char c, char *buf, size_t len, va_list ap) {
	return (-1);
}

static int format_irc_whois(char c, char *buf, size_t len, va_list ap) {
	return (-1);
}

static int (*const irc_format_handler[])(char, char *, size_t, va_list) = {
	format_irc_chat_info,
	format_irc_ctcp,
	format_irc_umode,
	format_irc_users,
	format_irc_whois,
};
