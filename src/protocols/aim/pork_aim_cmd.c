/*
** pork_aim_cmd.c - AIM specific user commands.
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
#include <pork_html.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_command.h>
#include <pork_conf.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

#include <libfaim/faimconfig.h>
#include <libfaim/aim.h>
#include <pork_aim.h>
#include <pork_aim_cmd.h>

/* XXX - todo */

static USER_COMMAND(aim_cmd_icon) {
}

static USER_COMMAND(aim_cmd_email) {
}

static USER_COMMAND(aim_cmd_passwd) {
}

static USER_COMMAND(aim_cmd_save) {
}

static USER_COMMAND(aim_cmd_idle_mode) {
	struct pork_acct *acct = cur_window()->owner;

	if (args != NULL && !blank_str(args)) {
		u_int32_t mode;

		if (str_to_uint(args, &mode) != 0) {
			screen_err_msg("Invalid number: %s", args);
			return;
		}

		acct->proto->set_report_idle(acct, mode);
	}

	screen_cmd_output("The reporting of idle time for %s is %s",
		acct->username, (acct->report_idle ? "enabled" : "disabled"));
}

static USER_COMMAND(aim_cmd_privacy_mode) {
	struct pork_acct *acct = cur_window()->owner;
	int mode = -1;

	if (args != NULL)
		str_to_int(args, &mode);

	mode = acct->proto->set_privacy_mode(acct, mode);
	screen_cmd_output("Privacy mode for %s is %d", acct->username, mode);
} 

static struct command aim_command[] = {
	{ "email",				aim_cmd_email			},
	{ "icon",				aim_cmd_icon			},
	{ "idle_mode",			aim_cmd_idle_mode		},
	{ "password",			aim_cmd_passwd			},
	{ "privacy_mode",		aim_cmd_privacy_mode	},
	{ "save",				aim_cmd_save			},
};

void aim_cmd_setup(struct pork_proto *proto) {
	proto->cmd = aim_command;
	proto->num_cmds = array_elem(aim_command);
}
