/*
** pork_perl_macro.h - Terrible, terrible things.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_PERL_MACRO_H
#define __PORK_PERL_MACRO_H

#define WIN_REFNUM(x, y) do { \
	(void) cv; \
	if (items > (x)) { \
		u_int32_t __refnum = SvIV(ST((x))); \
		struct imwindow *__win = imwindow_find_refnum(__refnum); \
		if (__win == NULL) \
			XSRETURN_##y; \
		else \
			win = __win; \
	} else \
		win = cur_window(); \
} while (0)

#define ACCT_WIN_REFNUM(x, y) do { \
	(void) cv; \
	if (items > (x)) { \
		u_int32_t __refnum = SvIV(ST((x))); \
		struct imwindow *__win = imwindow_find_refnum(__refnum); \
		if (__win == NULL) \
			XSRETURN_##y; \
		else \
			acct = __win->owner; \
	} else \
		acct = cur_window()->owner; \
} while (0)

#define INPUT_WIN_REFNUM(x, y) do { \
	(void) cv; \
	if (items > (x)) { \
		u_int32_t __refnum = SvIV(ST((x))); \
		struct imwindow *__win = imwindow_find_refnum(__refnum); \
		if (__win == NULL) \
			XSRETURN_##y; \
		else \
			input = __win->input; \
	} else \
		input = cur_window()->input; \
} while (0)

#else
#	warning "included multiple times"
#endif
