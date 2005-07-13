/*
** pork_aim_set.c - /aim set support.
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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_acct.h>
#include <pork_set.h>
#include <pork_aim_set.h>
#include <pork_proto.h>

/* XXX: TODO */

static const struct pork_pref aim_pref_list[] = {
	{   .name = "AUTOSAVE",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},
};

static const struct pref_set aim_pref_set = {
	.name = "aim",
	.num_opts = AIM_NUM_OPTS,
	.prefs = aim_pref_list
};

static pref_val_t aim_default_pref_vals[] = {
	{   .pref_val.b = DEFAULT_AIM_AUTOSAVE,
	}
};

static struct pref_val aim_defaults = {
	.set = &aim_pref_set,
	.val = aim_default_pref_vals
};

int aim_init_prefs(struct pork_acct *acct) {
	struct pref_val *prefs;

	prefs = xmalloc(sizeof(*prefs));
	prefs->set = &aim_pref_set;
	opt_copy_pref_val(prefs, aim_default_pref_vals,
		sizeof(aim_default_pref_vals));

	acct->proto_prefs = prefs;
	return (0);
}

inline struct pref_val *aim_get_default_prefs(void) {
	return (&aim_defaults);
}
