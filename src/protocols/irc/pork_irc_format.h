/*
** pork_irc_format.h
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef _PORK_IRC_FORMAT_H
#define _PORK_IRC_FORMAT_H

int irc_fill_format_str(struct pork_acct *acct, int type, char *buf, size_t len, ...);

#else
#	warn __FILE__ included multiple times.
#endif
