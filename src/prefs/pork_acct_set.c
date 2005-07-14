/*
** pork_acct_set.c - /acct set support
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

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_color.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_acct_set.h>
#include <pork_proto.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

static const struct pork_pref acct_pref_list[] = {
	{	.name = "ACCT_DIR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "AUTO_RECONNECT",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "AUTO_REJOIN",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "AUTOSAVE",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "AUTOSEND_AWAY",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "CONNECT_TIMEOUT",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "DOWNLOAD_DIR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "DUMP_MSGS_TO_STATUS",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "IDLE_AFTER",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "LOG_DIR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "LOGIN_ON_STARTUP",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "RECONNECT_INTERVAL",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "RECONNECT_MAX_INTERVAL",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "RECONNECT_TRIES",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "REPORT_IDLE",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "SAVE_PASSWD",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "SEND_REMOVES_AWAY",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "TRANSFER_PORT_MAX",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "TRANSFER_PORT_MIN",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	}
};

static const struct pref_set acct_pref_set = {
	.name = "acct",
	.num_opts = ACCT_NUM_OPTS,
	.prefs = acct_pref_list
};

static pref_val_t acct_default_pref_vals[] = {
	{	.pref_val.s = DEFAULT_ACCT_DIR,
	},{	.pref_val.i = DEFAULT_ACCT_AUTO_RECONNECT,
	},{	.pref_val.i = DEFAULT_ACCT_AUTO_REJOIN,
	},{	.pref_val.b = DEFAULT_ACCT_AUTOSAVE,
	},{	.pref_val.i = DEFAULT_ACCT_AUTOSEND_AWAY,
	},{	.pref_val.i = DEFAULT_ACCT_CONNECT_TIMEOUT,
	},{	.pref_val.s = DEFAULT_ACCT_DOWNLOAD_DIR,
	},{	.pref_val.b = DEFAULT_ACCT_DUMP_MSGS_TO_STATUS,
	},{	.pref_val.i = DEFAULT_ACCT_IDLE_AFTER,
	},{	.pref_val.s = DEFAULT_ACCT_LOG_DIR,
	},{	.pref_val.b = DEFAULT_ACCT_LOGIN_ON_STARTUP,
	},{	.pref_val.i = DEFAULT_ACCT_RECONNECT_INTERVAL,
	},{	.pref_val.i = DEFAULT_ACCT_RECONNECT_MAX_INTERVAL,
	},{	.pref_val.i = DEFAULT_ACCT_RECONNECT_TRIES,
	},{	.pref_val.b = DEFAULT_ACCT_REPORT_IDLE,
	},{	.pref_val.b = DEFAULT_ACCT_SAVE_PASSWD,
	},{	.pref_val.b = DEFAULT_ACCT_SEND_REMOVES_AWAY,
	},{	.pref_val.i = DEFAULT_ACCT_TRANSFER_PORT_MAX,
	},{	.pref_val.i = DEFAULT_ACCT_TRANSFER_PORT_MIN,
	}
};

static struct pref_val acct_defaults = {
	.set = &acct_pref_set,
	.val = acct_default_pref_vals
};

int acct_init_prefs(struct pork_acct *acct) {
	struct pref_val *prefs;
	char buf[4096];
	int ret;
	char *pork_dir;

	pork_dir = opt_get_str(screen.global_prefs, OPT_PORK_DIR);
	if (pork_dir == NULL) {
		pork_dir = DEFAULT_PORK_DIR;
		if (pork_dir == NULL)
			return (-1);
	}

	if (acct->proto->protocol >= 0) {
		ret = snprintf(buf, sizeof(buf), "%s%s/%s",
				pork_dir, acct->proto->name, acct->username);
	} else {
		ret = xstrncpy(buf, pork_dir, sizeof(buf));
	}

	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	prefs = xmalloc(sizeof(*prefs));
	prefs->set = &acct_pref_set;
	opt_copy_pref_val(prefs, acct_default_pref_vals,
		sizeof(acct_default_pref_vals));

	acct->prefs = prefs;

	opt_set(prefs, ACCT_OPT_ACCT_DIR, buf);

	if (xstrncat(buf, "/downloads", sizeof(buf)) == -1)
		return (-1);
	opt_set(prefs, ACCT_OPT_DOWNLOAD_DIR, buf);

	buf[strlen(buf) - 3] = '\0';

	if (xstrncat(buf, "/logs", sizeof(buf)) == -1)
		return (-1);
	opt_set(prefs, ACCT_OPT_LOG_DIR, buf);

	return (0);
}

inline struct pref_val *acct_get_default_prefs(void) {
	return (&acct_defaults);
}
