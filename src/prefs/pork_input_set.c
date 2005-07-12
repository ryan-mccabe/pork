/*
** pork_input_set.c - /input set support
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <pwd.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_color.h>
#include <pork_acct.h>
#include <pork_input.h>
#include <pork_set.h>
#include <pork_proto.h>
#include <pork_input_set.h>
#include <pork_imwindow.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

static void opt_updated_histlen(struct pref_val *pref, va_list ap) {
	struct pork_input *input;

	input = va_arg(ap, struct pork_input *);
	if (input != NULL)
		input_history_prune(input);
}

static void opt_updated_prompt(struct pref_val *pref, va_list ap) {
	struct pork_input *input;

	input = va_arg(ap, struct pork_input *);
	if (input != NULL) {
		char *prompt_str;

		prompt_str = opt_get_str(pref, INPUT_OPT_PROMPT_STR);
		if (prompt_str != NULL)
			input_set_prompt(input, prompt_str);
	}
}

static const struct pork_pref input_pref_list[] = {
	{	.name = "HISTORY_LEN",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
		.updated = opt_updated_histlen
	},{	.name = "PROMPT_STR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
		.updated = opt_updated_prompt
	}
};

static const struct pref_set input_pref_set = {
	.name = "input",
	.num_opts = INPUT_NUM_OPTS,
	.prefs = input_pref_list
};

static pref_val_t input_default_pref_vals[] = {
	{	.pref_val.i = DEFAULT_INPUT_HISTORY_LEN,
	},{	.pref_val.s = DEFAULT_INPUT_PROMPT_STR,
	}
};

static struct pref_val input_defaults = {
	.set = &input_pref_set,
	.val = input_default_pref_vals
};

int input_init_prefs(struct pork_input *input) {
	struct pref_val *prefs;

	prefs = xmalloc(sizeof(*prefs));
	prefs->set = &input_pref_set;
	opt_copy_pref_val(prefs, input_default_pref_vals,
		sizeof(input_default_pref_vals));

	input->prefs = prefs;

	return (0);
}

inline struct pref_val *input_get_default_prefs(void) {
	return (&input_defaults);
}
