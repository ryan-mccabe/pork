/*
** pork_perl_blist.c - Perl scripting support
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
#include <pork_acct.h>
#include <pork_imwindow.h>
#include <pork_slist.h>
#include <pork_buddy_list.h>
#include <pork_screen.h>
#include <pork_buddy.h>
#include <pork_perl_xs.h>

XS(PORK_blist_collapse) {
	struct pork_acct *acct;
	struct blist *blist;
	size_t notused;
	char *group;
	struct bgroup *bgroup;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	group = SvPV(ST(0), notused);
	if (group == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	bgroup = group_find(acct, group);
	if (bgroup == NULL)
		XSRETURN_IV(-1);

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	blist_collapse_group(blist, bgroup);
	XSRETURN_IV(0);
}

XS(PORK_blist_cursor) {
	struct pork_acct *acct;
	struct blist *blist;
	struct slist_cell *cell;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_EMPTY;

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_EMPTY;
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_EMPTY;

	cell = blist_get_cursor(blist);
	if (cell == NULL)
		XSRETURN_EMPTY;

	SP -= items;

	if (cell->type == TYPE_FLAT_CELL) {
		struct buddy *buddy = cell->data;

		XPUSHs(sv_2mortal(newSVpv(buddy->nname, 0)));
	} else {
		struct bgroup *group = cell->data;

		XPUSHs(sv_2mortal(newSVpv(group->name, 0)));
	}

	XPUSHs(sv_2mortal(newSViv(cell->type)));
	XSRETURN(2);
}

XS(PORK_blist_down) {
	struct pork_acct *acct;
	struct blist *blist;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(blist_cursor_down(blist));
}

XS(PORK_blist_end) {
	struct pork_acct *acct;
	struct blist *blist;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(blist_cursor_end(blist));
}

XS(PORK_blist_hide) {
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

	imwindow_blist_hide(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_blist_page_down) {
	struct pork_acct *acct;
	struct blist *blist;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(blist_cursor_pgdown(blist));
}

XS(PORK_blist_page_up) {
	struct pork_acct *acct;
	struct blist *blist;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(blist_cursor_pgup(blist));
}

XS(PORK_blist_refresh) {
	struct pork_acct *acct;
	struct blist *blist;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(blist_refresh(blist));
}

XS(PORK_blist_show) {
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

	imwindow_blist_show(imwindow);
	XSRETURN_IV(0);
}

XS(PORK_blist_start) {
	struct pork_acct *acct;
	struct blist *blist;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(blist_cursor_start(blist));
}

XS(PORK_blist_up) {
	struct pork_acct *acct;
	struct blist *blist;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(blist_cursor_up(blist));
}

XS(PORK_blist_width) {
	struct pork_acct *acct;
	struct blist *blist;
	int new_width;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	new_width = SvIV(ST(0));

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	blist = acct->blist;
	if (blist == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(screen_blist_width(blist, new_width));
}
