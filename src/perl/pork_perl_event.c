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
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_perl_xs.h>
#include <pork_perl_macro.h>

XS(PORK_event_add) {
	struct pork_acct *acct;
	char *type;
	char *handler;
	size_t notused;
	dXSARGS;
	u_int32_t refnum;

	if (items < 2)
		XSRETURN_UNDEF;

	type = SvPV(ST(0), notused);
	handler = SvPV(ST(1), notused);

	if (type == NULL || handler == NULL)
		XSRETURN_UNDEF;

	ACCT_WIN_REFNUM(2, UNDEF);
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

	if (items < 2)
		XSRETURN_IV(-1);

	type = SvPV(ST(0), notused);
	handler = SvPV(ST(1), notused);

	if (type == NULL || handler == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(2, IV(-1));
	XSRETURN_IV(event_del_type(acct->events, type, handler));
}

XS(PORK_event_del_type) {
	struct pork_acct *acct;
	size_t notused;
	char *type;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	type = SvPV(ST(0), notused);

	ACCT_WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(event_del_type(acct->events, type, NULL));
}

XS(PORK_event_del_refnum) {
	struct pork_acct *acct;
	u_int32_t refnum;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	refnum = SvIV(ST(0));

	ACCT_WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(event_del_refnum(acct->events, refnum));
}

XS(PORK_event_purge) {
	struct pork_acct *acct;
	dXSARGS;

	ACCT_WIN_REFNUM(0, IV(-1));
	event_purge(acct->events);
	XSRETURN_IV(0);
}
