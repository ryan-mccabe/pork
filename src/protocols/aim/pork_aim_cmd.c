/*
** pork_aim_cmd.c - AIM specific user commands.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
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
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_imsg.h>
#include <pork_io.h>
#include <pork_misc.h>
#include <pork_color.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_command.h>
#include <pork_conf.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

#include <libfaim/aim.h>

#include <pork_aim.h>
#include <pork_aim_html.h>
#include <pork_aim_proto.h>
#include <pork_aim_cmd.h>

/* XXX - todo */

USER_COMMAND(aim_cmd_icon) {
}

USER_COMMAND(aim_cmd_email) {
}

USER_COMMAND(aim_cmd_passwd) {
}

USER_COMMAND(aim_cmd_search) {
	if (args != NULL)
		aim_search(acct, args);
}

USER_COMMAND(aim_cmd_save) {
}

USER_COMMAND(aim_cmd_set) {
	if (acct == NULL)
		proto_set(proto_get(PROTO_AIM), NULL, args);
	else
		proto_set(acct->proto, acct->proto_prefs, args);
}

USER_COMMAND(aim_cmd_idle_mode) {
	if (args != NULL && !blank_str(args)) {
		u_int32_t mode;

		if (str_to_uint(args, &mode) != 0) {
			screen_err_msg(_("Invalid number: %s"), args);
			return;
		}

		aim_report_idle(acct, mode);
	}

	screen_cmd_output(_("The reporting of idle time for %s is %s"),
		acct->username, (acct->report_idle ? _("enabled") : _("disabled")));
}

USER_COMMAND(aim_cmd_privacy_mode) {
	int mode = -1;

	if (args != NULL)
		str_to_int(args, &mode);

	mode = aim_set_privacy_mode(acct, mode);
	screen_cmd_output(_("Privacy mode for %s is %d"), acct->username, mode);
}

static struct command aim_command[] = {
	{ "email",				aim_cmd_email			},
	{ "icon",				aim_cmd_icon			},
	{ "idle_mode",			aim_cmd_idle_mode		},
	{ "password",			aim_cmd_passwd			},
	{ "privacy_mode",		aim_cmd_privacy_mode	},
	{ "save",				aim_cmd_save			},
	{ "set",				aim_cmd_set				},
	{ "search",				aim_cmd_search			},
};

void aim_cmd_setup(struct pork_proto *proto) {
	proto->cmd = aim_command;
	proto->num_cmds = array_elem(aim_command);
}
