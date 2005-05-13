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
#include <pork_perl_macro.h>

XS(PORK_win_scroll_by) {
	int lines;
	struct imwindow *win;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	lines = SvIV(ST(0));

	WIN_REFNUM(1, IV(-1));
	imwindow_scroll_by(win, lines);
	XSRETURN_IV(0);
}

XS(PORK_win_scroll_down) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	imwindow_scroll_down(win);
	XSRETURN_IV(0);
}

XS(PORK_win_scroll_end) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	imwindow_scroll_end(win);
	XSRETURN_IV(0);
}

XS(PORK_win_scroll_page_down) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	imwindow_scroll_page_down(win);
	XSRETURN_IV(0);
}

XS(PORK_win_scroll_page_up) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	imwindow_scroll_page_up(win);
	XSRETURN_IV(0);
}

XS(PORK_win_scroll_start) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	imwindow_scroll_start(win);
	XSRETURN_IV(0);
}

XS(PORK_win_scroll_up) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	imwindow_scroll_up(win);
	XSRETURN_IV(0);
}

XS(PORK_win_find_name) {
	char *str;
	size_t notused;
	struct pork_acct *acct;
	struct imwindow *imwindow;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	str = SvPV(ST(0), notused);
	if (str == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
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

	if (items == 0)
		XSRETURN_PV(cur_window()->target);

	win_refnum = SvIV(ST(0));

	ACCT_WIN_REFNUM(1, UNDEF);
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

	if (items < 1)
		XSRETURN_IV(-1);

	str = SvPV(ST(0), notused);
	if (str == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	imwindow = imwindow_find(acct, str);
	if (imwindow == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(imwindow->refnum);
}

XS(PORK_win_bind) {
	u_int32_t acct_refnum;
	struct imwindow *win;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	acct_refnum = SvIV(ST(0));
	WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(imwindow_bind_acct(win, acct_refnum));
}

XS(PORK_win_clear) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	imwindow_clear(win);
	XSRETURN_IV(0);
}

XS(PORK_win_close) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	XSRETURN_IV(screen_close_window(win));
}

XS(PORK_win_erase) {
	struct imwindow *win;
	dXSARGS;

	WIN_REFNUM(0, IV(-1));
	imwindow_erase(win);
	XSRETURN_IV(0);
}

XS(PORK_win_next) {
	dXSARGS;

	(void) cv;
	(void) items;

	screen_cycle_fwd();
	XSRETURN_IV(0);
}

XS(PORK_win_prev) {
	dXSARGS;

	(void) cv;
	(void) items;

	screen_cycle_bak();
	XSRETURN_IV(0);
}

XS(PORK_win_rename) {
	dXSARGS;
	struct imwindow *win;
	char *new_name;
	size_t notused;

	if (items < 1)
		XSRETURN_IV(-1);

	new_name = SvPV(ST(0), notused);
	if (new_name == NULL)
		XSRETURN_IV(-1);

	WIN_REFNUM(1, IV(-1));
	imwindow_rename(win, new_name);
	XSRETURN_IV(0);
}

XS(PORK_win_renumber) {
	dXSARGS;
	struct imwindow *win;
	u_int32_t new_refnum;

	if (items < 1)
		XSRETURN_IV(-1);

	new_refnum = SvIV(ST(0));

	WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(screen_renumber(win, new_refnum));
}

XS(PORK_win_get_opt) {
	struct imwindow *win;
	char *val;
	size_t notused;
	char buf[2048];

	dXSARGS;

	if (items < 1)
		XSRETURN_EMPTY;

	val = SvPV(ST(0), notused);
	if (val == NULL)
		XSRETURN_EMPTY;

	WIN_REFNUM(1, EMPTY);
	if (wopt_get_val(win, val, buf, sizeof(buf)) == -1)
		XSRETURN_EMPTY;

	XSRETURN_PV(buf);
}

XS(PORK_win_set_opt) {
	char *var;
	char *val;
	size_t notused;
	int opt;
	struct imwindow *win;
	dXSARGS;

	if (items < 2)
		XSRETURN_IV(-1);

	var = SvPV(ST(0), notused);
	val = SvPV(ST(1), notused);

	if (var == NULL || val == NULL)
		XSRETURN_IV(-1);

	opt = wopt_find(var);
	if (opt == -1)
		XSRETURN_IV(-1);

	WIN_REFNUM(2, IV(-1));
	XSRETURN_IV(wopt_set(win, opt, val));
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
