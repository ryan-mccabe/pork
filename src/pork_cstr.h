/*
** pork_cstr.h - routines for dealing with strings of type chtype *.
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_CSTR_H
#define __PORK_CSTR_H

#define PORK_TABSTOP		4

#define chtype_set(x, c)	((x) = ((x) & ~A_CHARTEXT) | (c))
#define chtype_get(x)		((x) & A_CHARTEXT)
#define chtype_ctrl(x)		(((x) + 'A' - 1) | A_REVERSE)

struct pref_val;

size_t cstrlen(chtype *ch);
chtype *cstrndup(chtype *ch, size_t len);
char *cstr_to_plaintext(chtype *cstr, size_t n);

int plaintext_to_cstr(chtype *ch, size_t len, ...);
int plaintext_to_cstr_nocolor(chtype *ch, size_t len, ...);

size_t wputstr(WINDOW *win, struct pref_val *, chtype *ch);
size_t wputnstr(WINDOW *win, struct pref_val *, chtype *ch, size_t n);
size_t mvwputstr(WINDOW *win, struct pref_val *,int y, int x, chtype *ch);
size_t mvwputnstr(WINDOW *win, struct pref_val *, int y, int x, chtype *ch, size_t n);
size_t wputncstr(WINDOW *win, char *str, size_t n);

#else
#	warning "included multiple times"
#endif
