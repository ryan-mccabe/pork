/*
** pork_perl_timer.c - Perl scripting support
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
#include <pork_timer.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_perl_xs.h>

XS(PORK_timer_add) {
	char *command;
	u_int32_t interval;
	u_int32_t times;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 3)
		XSRETURN_UNDEF;

	interval = SvIV(ST(0));
	times = SvIV(ST(1));
	command = SvPV(ST(2), notused);

	if (command == NULL)
		XSRETURN_UNDEF;

	XSRETURN_IV(timer_add(&screen.timer_list, command, cur_window()->owner, interval, times));
}

XS(PORK_timer_del) {
	char *command;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_IV(-1);

	command = SvPV(ST(0), notused);
	if (command == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(timer_del(&screen.timer_list, command));
}

XS(PORK_timer_del_refnum) {
	u_int32_t refnum;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_IV(-1);

	refnum = SvIV(ST(0));
	XSRETURN_IV(timer_del_refnum(&screen.timer_list, refnum));
}

XS(PORK_timer_purge) {
	dXSARGS;

	(void) items;
	(void) cv;

	timer_destroy(&screen.timer_list);
	XSRETURN_IV(0);
}
