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
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_set.h>
#include <pork_aim_set.h>
#include <pork_proto.h>

#include <libfaim/faimconfig.h>

static const struct pork_pref aim_pref_list[] = {
	{	.name = "AUTOSAVE",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "FORMAT_WHOIS_AWAY",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WHOIS_IDLE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WHOIS_MEMBER",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WHOIS_NAME",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WHOIS_SIGNON",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WHOIS_USERINFO",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_WHOIS_WARNLEVEL",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "OUTGOING_MSG_FONT",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "OUTGOING_MSG_FONT_BGCOLOR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "OUTGOING_MSG_FONT_FGCOLOR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "OUTGOING_MSG_FONT_SIZE",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "PORT",
		.type = OPT_TYPE_STR,
		.set = opt_set_str
	},{	.name = "SERVER",
		.type = OPT_TYPE_STR,
		.set = opt_set_str
	}
};

static const struct pref_set aim_pref_set = {
	.name = "aim",
	.num_opts = AIM_NUM_OPTS,
	.prefs = aim_pref_list
};

static pref_val_t aim_default_pref_vals[] = {
	{	.pref_val.b = DEFAULT_AIM_AUTOSAVE,
    },{	.pref_val.s = DEFAULT_AIM_FORMAT_WHOIS_AWAY,
    },{	.pref_val.s = DEFAULT_AIM_FORMAT_WHOIS_IDLE,
    },{	.pref_val.s = DEFAULT_AIM_FORMAT_WHOIS_MEMBER,
    },{	.pref_val.s = DEFAULT_AIM_FORMAT_WHOIS_NAME,
    },{	.pref_val.s = DEFAULT_AIM_FORMAT_WHOIS_SIGNON,
    },{	.pref_val.s = DEFAULT_AIM_FORMAT_WHOIS_USERINFO,
    },{	.pref_val.s = DEFAULT_AIM_FORMAT_WHOIS_WARNLEVEL,
    },{	.pref_val.s = DEFAULT_AIM_OUTGOING_MSG_FONT,
    },{	.pref_val.s = DEFAULT_AIM_OUTGOING_MSG_FONT_BGCOLOR,
    },{	.pref_val.s = DEFAULT_AIM_OUTGOING_MSG_FONT_FGCOLOR,
    },{	.pref_val.s = DEFAULT_AIM_OUTGOING_MSG_FONT_SIZE
	},{	.pref_val.s = DEFAULT_AIM_PORT,
	},{	.pref_val.s = DEFAULT_AIM_SERVER,
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
