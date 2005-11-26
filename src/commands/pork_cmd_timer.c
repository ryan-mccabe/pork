/*
** pork_cmd_timer.c - /timer commands.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
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
#include <ncurses.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_set.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_imsg.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_timer.h>
#include <pork_command.h>

static void print_timer(void *data, void *nothing __notused) {
	struct timer_entry *timer = data;

	screen_cmd_output("[refnum: %u] %d %u %s", timer->refnum,
		(int) timer->interval, timer->times, timer->command);
}

USER_COMMAND(cmd_timer_add) {
	char *p;
	u_int32_t interval;
	u_int32_t times;

	if (args == NULL)
		return;

	p = strsep(&args, " ");
	if (p == NULL)
		return;

	if (str_to_uint(p, &interval) != 0) {
		screen_err_msg("Invalid timer interval: %s", p);
		return;
	}

	p = strsep(&args, " ");
	if (p == NULL)
		return;

	if (str_to_uint(p, &times) != 0) {
		screen_err_msg("Invalid number of times to run: %s", p);
		return;
	}

	if (args == NULL || blank_str(args))
		return;

	timer_add(&screen.timer_list, args, acct, interval, times);
}

USER_COMMAND(cmd_timer_del) {
	int ret;

	if (args == NULL)
		return;

	ret = timer_del(&screen.timer_list, args);
	if (ret == -1)
		screen_err_msg("No timer for \"%s\" was found", args);
	else
		screen_cmd_output("Timer for \"%s\" was removed", args);
}

USER_COMMAND(cmd_timer_del_refnum) {
	u_int32_t refnum;
	int ret;

	if (args == NULL)
		return;

	if (str_to_uint(args, &refnum) != 0) {
		screen_err_msg("Bad timer refnum: %s", args);
		return;
	}

	ret = timer_del_refnum(&screen.timer_list, refnum);
	if (ret == -1)
		screen_err_msg("No timer with refnum %u was found", refnum);
	else
		screen_cmd_output("Timer with refnum %u was removed", refnum);
}

USER_COMMAND(cmd_timer_list) {
	dlist_iterate(screen.timer_list, print_timer, NULL);
}

USER_COMMAND(cmd_timer_purge) {
	if (screen.timer_list != NULL) {
		timer_destroy(&screen.timer_list);
		screen_cmd_output("All timers have been removed");
	}
}

static struct command timer_command[] = {
	{ "add",			cmd_timer_add			},
	{ "del",			cmd_timer_del			},
	{ "del_refnum",		cmd_timer_del_refnum	},
	{ "list",			cmd_timer_list			},
	{ "purge",			cmd_timer_purge			},
};

struct command_set timer_set = { timer_command, array_elem(timer_command), "timer " };
