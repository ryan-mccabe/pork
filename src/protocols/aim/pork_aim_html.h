/*
** pork_aim_html.h - functions for dealing with AIM HTML
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_AIM_HTML_H
#define __PORK_AIM_HTML_H

char *strip_html(char *str);
char *text_to_html(const char *in);

#else
#	warning "included multiple times"
#endif
