/*
** pork_perl_win.c - Perl scripting support
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdlib.h>
#include <sys/mman.h>

#undef PACKAGE
#undef instr

#ifndef HAS_BOOL
#	define HAS_BOOL
#endif

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_set.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_acct.h>
#include <pork_screen.h>
#include <pork_perl_xs.h>

XS(PORK_scroll_by) {
	int lines;
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	lines = SvIV(ST(0));

	if (items == 2) {
		u_int32_t refnum = SvIV(ST(1));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_scroll_by(imwindow, lines);
	XSRETURN_IV(0);
}

XS(PORK_scroll_down) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_scroll_down(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_scroll_end) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_scroll_end(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_scroll_page_down) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_scroll_page_down(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_scroll_page_up) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_scroll_page_up(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_scroll_start) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_scroll_start(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_scroll_up) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_scroll_up(imwindow);
	XSRETURN_IV(0);
}

/*
** window commands
*/

XS(PORK_win_find_name) {
	char *str;
	size_t notused;
	struct pork_acct *acct;
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	str = SvPV(ST(0), notused);
	if (str == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	imwindow = imwindow_find_name(acct, str);
	if (imwindow == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(imwindow->refnum);
}

XS(PORK_win_target) {
	u_int32_t win_refnum;
	struct imwindow *imwindow;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items == 0)
		XSRETURN_PV(cur_window()->target);

	win_refnum = SvIV(ST(0));

	if (items > 1) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_UNDEF;
	} else
		acct = cur_window()->owner;

	imwindow = imwindow_find_refnum(win_refnum);
	if (imwindow == NULL)
		XSRETURN_UNDEF;

	XSRETURN_PV(imwindow->target);
}

XS(PORK_win_find_target) {
	char *str;
	size_t notused;
	struct imwindow *imwindow;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	str = SvPV(ST(0), notused);
	if (str == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	imwindow = imwindow_find(acct, str);
	if (imwindow == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(imwindow->refnum);
}

XS(PORK_win_bind) {
	u_int32_t acct_refnum;
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	acct_refnum = SvIV(ST(0));

	if (items == 2) {
		u_int32_t refnum = SvIV(ST(1));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	XSRETURN_IV(imwindow_bind_acct(imwindow, acct_refnum));
}

XS(PORK_win_clear) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_clear(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_win_close) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	XSRETURN_IV(screen_close_window(imwindow));
}

XS(PORK_win_erase) {
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t refnum = SvIV(ST(0));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_erase(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_win_next) {
	dXSARGS;

	(void) cv;

	if (items != 0)
		XSRETURN_IV(-1);

	screen_cycle_fwd();
	XSRETURN_IV(0);
}

XS(PORK_win_prev) {
	dXSARGS;

	(void) cv;

	if (items != 0)
		XSRETURN_IV(-1);

	screen_cycle_bak();
	XSRETURN_IV(0);
}

XS(PORK_win_rename) {
	dXSARGS;
	struct imwindow *imwindow;
	char *new_name;
	size_t notused;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	new_name = SvPV(ST(0), notused);
	if (new_name == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t refnum = SvIV(ST(1));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	imwindow_rename(imwindow, new_name);
	XSRETURN_IV(0);
}

XS(PORK_win_renumber) {
	dXSARGS;
	struct imwindow *imwindow;
	u_int32_t new_refnum;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	new_refnum = SvIV(ST(0));

	if (items == 2) {
		u_int32_t refnum = SvIV(ST(1));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	XSRETURN_IV(screen_renumber(imwindow, new_refnum));
}

XS(PORK_win_get_opt) {
	struct imwindow *imwindow;
	char *val;
	size_t notused;
	char buf[2048];

	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_EMPTY;

	val = SvPV(ST(0), notused);
	if (val == NULL)
		XSRETURN_EMPTY;

	if (items == 2) {
		u_int32_t refnum = SvIV(ST(1));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_EMPTY;
	} else
		imwindow = cur_window();

	if (wopt_get_val(imwindow, val, buf, sizeof(buf)) == -1)
		XSRETURN_EMPTY;

	XSRETURN_PV(buf);
}

XS(PORK_win_set_opt) {
	char *var;
	char *val;
	size_t notused;
	int opt;
	struct imwindow *imwindow;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	var = SvPV(ST(0), notused);
	val = SvPV(ST(1), notused);

	if (var == NULL || var == NULL)
		XSRETURN_IV(-1);

	opt = wopt_find(var);
	if (opt == -1)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t refnum = SvIV(ST(2));

		imwindow = imwindow_find_refnum(refnum);
		if (imwindow == NULL)
			XSRETURN_IV(-1);
	} else
		imwindow = cur_window();

	XSRETURN_IV(wopt_set(imwindow, opt, val));
}

XS(PORK_win_swap) {
	u_int32_t refnum;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_IV(-1);

	refnum = SvIV(ST(0));
	XSRETURN_IV(screen_goto_window(refnum));
}
