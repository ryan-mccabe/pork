/*
** pork_format.h - parsing format strings.
** Copyright (C) 2003-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_FORMAT_H
#define __PORK_FORMAT_H

#define FORMAT_VARIABLE '$'

extern int (*const global_format_handler[])(char, char *, size_t, va_list);

int fill_format_string(	int type,
						char *buf,
						size_t len,
						struct pref_val *prefs,
						int (*handler)(char, char *, size_t, va_list), ...);

void format_apply_justification(char *buf, chtype *ch, size_t len);

#define fill_format_str(type, buf, len, args...) fill_format_string((type), (buf), (len), screen.global_prefs, global_format_handler[(type) - OPT_FORMAT_OFFSET], ##args)

#else
#	warning "included multiple times"
#endif
