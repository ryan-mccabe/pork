/*
** pork_cmd_win.c - /win commands.
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
#include <pork_imwindow_set.h>
#include <pork_command.h>

USER_COMMAND(cmd_win_bind) {
	struct imwindow *imwindow = cur_window();
	u_int32_t refnum;
	int ret;

	if (blank_str(args)) {
		if (imwindow->owner != NULL && imwindow->owner->username != NULL) {
			screen_cmd_output(_("This window is bound to account %s [refnum %u]"),
				imwindow->owner->username, imwindow->owner->refnum);
		} else
			screen_cmd_output(_("This window is bound to no account"));

		return;
	}

	if (str_to_uint(args, &refnum) == -1) {
		screen_err_msg(_("Bad account refnum: %s"), args);
		return;
	}

	ret = imwindow_bind_acct(imwindow, refnum);
	if (ret == -1) {
		if (imwindow->type == WIN_TYPE_CHAT)
			screen_err_msg(_("You can't rebind chat windows"));
		else
			screen_err_msg(_("Account refnum %s doesn't exist"), args);
	} else {
		screen_cmd_output(_("This window is now bound to account %s [refnum %u]"),
			imwindow->owner->username, imwindow->owner->refnum);
	}
}

USER_COMMAND(cmd_win_bind_next) {
	if (imwindow_bind_next_acct(cur_window()) != -1)
		screen_refresh();
}

USER_COMMAND(cmd_win_clear) {
	imwindow_clear(cur_window());
}

USER_COMMAND(cmd_win_close) {
	screen_close_window(cur_window());
}

USER_COMMAND(cmd_win_dump) {
	if (blank_str(args)) {
		screen_err_msg(_("No output file specified"));
	} else {
		char buf[4096];

		expand_path(args, buf, sizeof(buf));
		imwindow_dump_buffer(cur_window(), buf);
	}
}

USER_COMMAND(cmd_win_erase) {
	imwindow_erase(cur_window());
}

USER_COMMAND(cmd_win_ignore) {
	struct imwindow *win;

	if (!blank_str(args)) {
		u_int32_t refnum;

		if (str_to_uint(args, &refnum) != 0) {
			screen_err_msg(_("Bad window refnum: %s"), args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg(_("No window with refnum %u"), refnum);
			return;
		}
	} else
		win = cur_window();

	imwindow_ignore(win);
}

USER_COMMAND(cmd_win_list) {
	dlist_t *cur;
	static const char *win_types[] = { "Status", "Conv", "Chat" };

	screen_cmd_output(_("Window List:"));

	screen_cmd_output(_("REFNUM\t\tNAME\t\tTYPE\t\tTARGET"));
	cur = screen.window_list;
	do {
		struct imwindow *imwindow = cur->data;

		screen_cmd_output("%u\t\t\t%s\t\t%s\t\t%s",
			imwindow->refnum, imwindow->name,
			win_types[imwindow->type], imwindow->target);

		cur = cur->next;
	} while (cur != screen.window_list);
}

USER_COMMAND(cmd_win_next) {
	screen_cycle_fwd();
}

USER_COMMAND(cmd_win_next_active) {
	screen_cycle_fwd_active();
}

USER_COMMAND(cmd_win_prev) {
	screen_cycle_bak();
}

USER_COMMAND(cmd_win_prev_active) {
	screen_cycle_bak_active();
}

USER_COMMAND(cmd_win_rename) {
	struct imwindow *win = cur_window();

	if (args == NULL)
		screen_cmd_output(_("Window %u has name \"%s\""), win->refnum, win->name);
	else
		imwindow_rename(win, args);
}

USER_COMMAND(cmd_win_renumber) {
	u_int32_t num;

	if (blank_str(args)) {
		screen_cmd_output(_("This is window %u"), cur_window()->refnum);
		return;
	}

	if (str_to_uint(args, &num) != 0) {
		screen_err_msg(_("Bad window number: %s"), args);
		return;
	}

	screen_renumber(cur_window(), num);
}

USER_COMMAND(cmd_win_set) {
	struct imwindow *win;
	struct pref_val *pref;

	if (blank_str(args)) {
		win = cur_window();
		pref = win->prefs;
	} else if (!strncasecmp(args, "-default", 8)) {
		args += 8;
		while (args[0] == ' ')
			args++;

		win = NULL;
		pref = imwindow_get_default_prefs();
	} else if (!strncasecmp(args, "-refnum", 7)) {
		u_int32_t refnum;
		char *refnum_str;

		args += 7;
		refnum_str = strsep(&args, " ");
		if (refnum_str == NULL || str_to_uint(args, &refnum) != 0) {
			screen_err_msg(_("Invalid window refnum: %s"), args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg(_("No window with refnum %u exists"), refnum);
			return;
		}

		pref = win->prefs;
	} else {
		win = cur_window();
		pref = win->prefs;
	}

	opt_set_var(pref, args, win);
}

USER_COMMAND(cmd_win_skip) {
	struct imwindow *win;

	if (!blank_str(args)) {
		u_int32_t refnum;

		if (str_to_uint(args, &refnum) != 0) {
			screen_err_msg(_("Bad window refnum: %s"), args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg(_("No window with refnum %u"), refnum);
			return;
		}
	} else
		win = cur_window();

	imwindow_skip(win);
}

USER_COMMAND(cmd_win_swap) {
	u_int32_t num;

	if (blank_str(args))
		return;

	if (str_to_uint(args, &num) != 0) {
		screen_err_msg(_("Invalid window refnum: %s"), args);
		return;
	}

	if (screen_goto_window(num) != 0)
		screen_err_msg(_("No such window: %s"), args);
}

USER_COMMAND(cmd_win_unignore) {
	struct imwindow *win;

	if (!blank_str(args)) {
		u_int32_t refnum;

		if (str_to_uint(args, &refnum) != 0) {
			screen_err_msg(_("Bad window refnum: %s"), args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg(_("No window with refnum %u"), refnum);
			return;
		}
	} else
		win = cur_window();

	imwindow_unignore(win);
}

USER_COMMAND(cmd_win_unskip) {
	struct imwindow *win;

	if (!blank_str(args)) {
		u_int32_t refnum;

		if (str_to_uint(args, &refnum) != 0) {
			screen_err_msg(_("Bad window refnum: %s"), args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg(_("No window with refnum %u"), refnum);
			return;
		}
	} else
		win = cur_window();

	imwindow_unskip(win);
}

static struct command win_command[] = {
	{ "bind",				cmd_win_bind		},
	{ "bind_next",			cmd_win_bind_next	},
	{ "clear",				cmd_win_clear		},
	{ "close",				cmd_win_close		},
	{ "dump",				cmd_win_dump		},
	{ "erase",				cmd_win_erase		},
	{ "ignore",				cmd_win_ignore		},
	{ "list",				cmd_win_list		},
	{ "next",				cmd_win_next		},
	{ "next_active",		cmd_win_next_active	},
	{ "prev",				cmd_win_prev		},
	{ "prev_active",		cmd_win_prev_active	},
	{ "rename",				cmd_win_rename		},
	{ "renumber",			cmd_win_renumber	},
	{ "set",				cmd_win_set			},
	{ "skip",				cmd_win_skip		},
	{ "swap",				cmd_win_swap		},
	{ "unignore",			cmd_win_unignore	},
	{ "unskip",				cmd_win_unskip		},
};

struct command_set win_set = { win_command, array_elem(win_command), "win " };
