/*
** pork_perl_input.c - Perl scripting support
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
#include <pork_input.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_command.h>
#include <pork_command_defs.h>
#include <pork_perl_xs.h>

XS(PORK_input_send) {
	char *args;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items == 0)
		XSRETURN_IV(-1);

	args = SvPV(ST(0), notused);

	if (args == NULL)
		XSRETURN_IV(-1);

	cmd_input_send(args);
	XSRETURN_IV(0);
}

XS(PORK_input_get_data) {
	dXSARGS;

	(void) cv;
	(void) items;

	XSRETURN_PV(input_get_buf_str(cur_window()->input));
}

