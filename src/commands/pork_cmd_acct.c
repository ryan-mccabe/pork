/*
** pork_cmd_win.c - /acct commands.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_set.h>
#include <pork_acct_set.h>
#include <pork_screen_io.h>
#include <pork_command.h>

USER_COMMAND(cmd_acct_list) {
	pork_acct_print_list();
}

USER_COMMAND(cmd_acct_save) {
	pork_acct_save(acct);
}

USER_COMMAND(cmd_acct_set) {
	struct pref_val *pref;

	if (blank_str(args))
		pref = acct->prefs;
	else if (!strncasecmp(args, "-default", 8)) {
		args += 8;
		while (args[0] == ' ')
			args++;

		acct = NULL;
		pref = acct_get_default_prefs();
	} else if (!strncasecmp(args, "-refnum", 7)) {
		u_int32_t refnum;
		char *refnum_str;

		args += 7;
		refnum_str = strsep(&args, " ");
		if (refnum_str == NULL || str_to_uint(args, &refnum) != 0) {
			screen_err_msg(_("Invalid account refnum: %s"), args);
			return;
		}

		acct = pork_acct_get_data(refnum);
		if (acct == NULL) {
			screen_err_msg(_("No account with refnum %u exists"), refnum);
			return;
		}

		pref = acct->prefs;
	} else
		pref = acct->prefs;

	opt_set_var(pref, args, acct);
}

static struct command acct_command[] = {
	{ "list",	cmd_acct_list		},
	{ "save",	cmd_acct_save		},
	{ "set",	cmd_acct_set		},
};

struct command_set acct_set = { acct_command, array_elem(acct_command), "acct " };
