/*
** pork_irc_format.h
** Copyright (C) 2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef _PORK_IRC_FORMAT_H
#define _PORK_IRC_FORMAT_H

extern int (*const irc_format_handler[])(char, char *, size_t, va_list);
#define irc_fill_format_str(acct, type, buf, len, args...) fill_format_string((type), (buf), (len), (acct)->proto_prefs, irc_format_handler[(type) - IRC_OPT_FORMAT_OFFSET], ##args);

#else
#	warn __FILE__ included multiple times.
#endif
