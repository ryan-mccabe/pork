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
#include <pwd.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_color.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_acct_set.h>
#include <pork_set.h>
#include <pork_imwindow.h>
#include <pork_acct.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

static const struct pork_pref acct_pref_list[] = {
	{	"AUTO_RECONNECT",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"AUTO_REJOIN",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"AUTOSEND_AWAY",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"CONNECT_TIMEOUT",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	},{	"DOWNLOAD_DIR",
		OPT_TYPE_STR,
		opt_set_str,
		NULL
	},{	"DUMP_MSGS_TO_STATUS",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"IDLE_AFTER",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	},{	"LOG_DIR",
		OPT_TYPE_STR,
		opt_set_str,
		NULL
	},{	"LOGIN_ON_STARTUP",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"PORK_DIR",
		OPT_TYPE_STR,
		opt_set_str,
		NULL
	},{	"RECONNECT_INTERVAL",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	},{	"RECONNECT_MAX_INTERVAL",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	},{	"RECONNECT_TRIES",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	},{	"REPORT_IDLE",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"SAVE_PASSWD",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"SEND_REMOVES_AWAY",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"TRANSFER_PORT_MAX",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	},{	"TRANSFER_PORT_MIN",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	}
};

static const struct pref_set acct_pref_set = {
	.name = "acct",
	.num_opts = ACCT_NUM_OPTS,
	.prefs = acct_pref_list
};

static pref_val_t acct_default_pref_vals[] = {
	{	.pref_val.i = DEFAULT_ACCT_AUTO_RECONNECT,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_AUTO_REJOIN,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_AUTOSEND_AWAY,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_CONNECT_TIMEOUT,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_ACCT_DOWNLOAD_DIR,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_ACCT_DUMP_MSGS_TO_STATUS,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_IDLE_AFTER,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_ACCT_LOG_DIR,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_ACCT_LOGIN_ON_STARTUP,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_ACCT_PORK_DIR,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_RECONNECT_INTERVAL,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_RECONNECT_MAX_INTERVAL,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_RECONNECT_TRIES,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_ACCT_REPORT_IDLE,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_ACCT_SAVE_PASSWD,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_ACCT_SAVE_PASSWD,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_ACCT_SEND_REMOVES_AWAY,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_TRANSFER_PORT_MAX,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_ACCT_TRANSFER_PORT_MIN,
		.dynamic = 0
	}
};

static struct pref_val acct_defaults = {
	.set = &acct_pref_set,
	.val = acct_default_pref_vals
};

int acct_init_prefs(struct pork_acct *acct) {
	struct pref_val *prefs;
	struct passwd *pw;
	char buf[4096];
	int ret;

	pw = getpwuid(getuid());
	if (pw == NULL)
		return (-1);

	if (acct->proto->protocol >= 0) {
		ret = snprintf(buf, sizeof(buf), "%s/.pork/%s/%s",
				pw->pw_dir, acct->proto->name, acct->username);
	} else {
		ret = snprintf(buf, sizeof(buf), "%s/.pork", pw->pw_dir);
	}

	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	prefs = xmalloc(sizeof(acct_defaults));
	memcpy(prefs, &acct_defaults, sizeof(acct_defaults));
	acct->prefs = prefs;

	opt_set(prefs, ACCT_OPT_PORK_DIR, buf);

	if (xstrncat(buf, "/dl", sizeof(buf) == -1))
		return (-1);
	opt_set(prefs, ACCT_OPT_DOWNLOAD_DIR, buf);

	buf[strlen(buf) - 3] = '\0';
	if (xstrncat(buf, "/logs", sizeof(buf) == -1))
		return (-1);
	opt_set(prefs, ACCT_OPT_LOG_DIR, buf);

	return (0);
}
