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
#include <pork_perl_macro.h>

XS(PORK_input_send) {
	char *args;
	size_t notused;
	struct pork_input *input;
	dXSARGS;

	if (items == 0)
		XSRETURN_IV(-1);

	args = SvPV(ST(0), notused);
	if (args == NULL)
		XSRETURN_IV(-1);

	INPUT_WIN_REFNUM(1, IV(-1));
	cmd_input_send(cur_window()->owner, args);
	XSRETURN_IV(0);
}

XS(PORK_input_get_data) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, UNDEF);
	XSRETURN_PV(input_get_buf_str(input));
}

XS(PORK_input_bkspace) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_bkspace(input);
	XSRETURN_IV(0);
}

XS(PORK_input_clear) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_clear_line(input);
	XSRETURN_IV(0);
}

XS(PORK_input_clear_prev_word) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_clear_prev_word(input);
	XSRETURN_IV(0);
}

XS(PORK_input_clear_next_word) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_clear_next_word(input);
	XSRETURN_IV(0);
}

XS(PORK_input_clear_to_end) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_clear_to_end(input);
	XSRETURN_IV(0);
}

XS(PORK_input_clear_to_start) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_clear_to_start(input);
	XSRETURN_IV(0);
}

XS(PORK_input_delete) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_delete(input);
	XSRETURN_IV(0);
}

XS(PORK_input_insert) {
	char *str;
	size_t notused;
	struct pork_input *input;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	str = SvPV(ST(0), notused);
	if (str == NULL)
		XSRETURN_IV(-1);

	INPUT_WIN_REFNUM(1, IV(-1));
	input_insert_str(input, str);
	XSRETURN_IV(0);
}

XS(PORK_input_move_cursor_left) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_move_left(input);
	XSRETURN_IV(0);
}

XS(PORK_input_move_cursor_right) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_move_right(input);
	XSRETURN_IV(0);
}

XS(PORK_input_move_cursor_prev_word) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_prev_word(input);
	XSRETURN_IV(0);
}

XS(PORK_input_move_cursor_next_word) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_next_word(input);
	XSRETURN_IV(0);
}

XS(PORK_input_move_cursor_start) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_home(input);
	XSRETURN_IV(0);
}

XS(PORK_input_move_cursor_end) {
	struct pork_input *input;
	dXSARGS;

	INPUT_WIN_REFNUM(0, IV(-1));
	input_end(input);
	XSRETURN_IV(0);
}

XS(PORK_input_remove) {
	int num_to_remove;
	struct pork_input *input;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	num_to_remove = SvIV(ST(0));
	if (num_to_remove == 0)
		XSRETURN_IV(0);

	INPUT_WIN_REFNUM(1, IV(-1));
	input_remove(input, num_to_remove);
	XSRETURN_IV(0);
}
