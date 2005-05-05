/*
** pork_irc_cmd.c - IRC specific user commands.
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

#include <pork_irc.h>
#include <pork_irc_cmd.h>

/* XXX - todo */

static USER_COMMAND(irc_cmd_kill) {
} 

static USER_COMMAND(irc_cmd_save) {
} 

static struct command irc_command[] = {
	{ "kill",				irc_cmd_kill	},
	{ "save",				irc_cmd_save	},
};

void irc_cmd_setup(struct pork_proto *proto) {
	proto->cmd = irc_command;
	proto->num_cmds = array_elem(irc_command);
}
