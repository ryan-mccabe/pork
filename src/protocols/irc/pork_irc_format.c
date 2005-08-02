/*
** pork_irc_format.c
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

/* XXX - FIXME */

static int (*const irc_format_handler[])(char, char *, size_t, va_list) = {
	format_irc_whois,
}
