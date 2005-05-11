/*
** pork_perl_event.c - Perl scripting support
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
#include <pork_events.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_perl_xs.h>

XS(PORK_event_add) {
	struct pork_acct *acct;
	char *type;
	char *handler;
	size_t notused;
	dXSARGS;
	u_int32_t refnum;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_UNDEF;

	type = SvPV(ST(0), notused);
	handler = SvPV(ST(1), notused);

	if (type == NULL || handler == NULL)
		XSRETURN_UNDEF;

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_UNDEF;
	} else
		acct = cur_window()->owner;

	if (event_add(acct->events, type, handler, &refnum) != 0)
		XSRETURN_UNDEF;

	XSRETURN_IV(refnum);
}

XS(PORK_event_del) {
	struct pork_acct *acct;
	size_t notused;
	char *type;
	char *handler;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	type = SvPV(ST(0), notused);
	handler = SvPV(ST(1), notused);

	if (type == NULL || handler == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(event_del_type(acct->events, type, handler));
}

XS(PORK_event_del_type) {
	struct pork_acct *acct;
	size_t notused;
	char *type;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	type = SvPV(ST(0), notused);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(event_del_type(acct->events, type, NULL));
}

XS(PORK_event_del_refnum) {
	struct pork_acct *acct;
	u_int32_t refnum;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	refnum = SvIV(ST(0));

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(event_del_refnum(acct->events, refnum));
}

XS(PORK_event_purge) {
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	event_purge(acct->events);
	XSRETURN_IV(0);
}
