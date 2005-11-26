/*
** pork_blist_cmd.c - /blist commands
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
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_buddy.h>
#include <pork_slist.h>
#include <pork_buddy_list.h>
#include <pork_msg.h>
#include <pork_command.h>

USER_COMMAND(cmd_blist_add_block) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL || cell->data == NULL)
		return;

	buddy = cell->data;
	if (buddy_add_block(acct, buddy->nname, 1) == -1) {
		screen_err_msg("%s is already on %s's block list",
			buddy->name, acct->username);
	} else {
		screen_cmd_output("%s has been added to %s's blocked users list",
			buddy->name, acct->username);
	}
}

USER_COMMAND(cmd_blist_add_permit) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL || cell->data == NULL)
		return;

	buddy = cell->data;
	if (buddy_add_permit(acct, buddy->nname, 1) == -1) {
		screen_err_msg("%s is already on %s's permit list",
			buddy->name, acct->username);
	} else {
		screen_cmd_output("%s has been added to %s's permitted users list",
			buddy->name, acct->username);
	}
}

USER_COMMAND(cmd_blist_away) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL || cell->data == NULL)
		return;

	buddy = cell->data;
	if (acct->proto->get_away_msg == NULL)
		acct->proto->get_away_msg(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_collapse) {
	struct slist_cell *cell;
	struct blist *blist = acct->blist;

	if (args == NULL)
		cell = blist_get_cursor(blist);
	else {
		dlist_t *node;
		struct bgroup *gr = group_find(acct, args);

		if (gr == NULL)
			return;

		node = gr->blist_line;
		if (node == NULL || node->data == NULL)
			return;
		cell = node->data;
	}

	if (cell == NULL || cell->type != TYPE_LIST_CELL)
		return;

	blist_collapse_group(blist, cell->data);
}

USER_COMMAND(cmd_blist_down) {
	blist_cursor_down(acct->blist);
}

USER_COMMAND(cmd_blist_end) {
	blist_cursor_end(acct->blist);
}

USER_COMMAND(cmd_blist_goto) {
	char *target;

	if (args != NULL)
		target = args;
	else {
		struct blist *blist = acct->blist;
		struct slist_cell *cell;
		struct buddy *buddy;

		cell = blist_get_cursor(blist);
		if (cell == NULL || cell->type == TYPE_LIST_CELL || cell->data == NULL)
			return;

		buddy = cell->data;
		target = buddy->nname;
	}

	/* XXX FIXME
	cmd_query(acct, target);
	*/
}

USER_COMMAND(cmd_blist_hide) {
	imwindow_blist_hide(cur_window());
}

USER_COMMAND(cmd_blist_pgdown) {
	blist_cursor_pgdown(acct->blist);
}

USER_COMMAND(cmd_blist_pgup) {
	blist_cursor_pgup(acct->blist);
}

USER_COMMAND(cmd_blist_profile) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL || cell->data == NULL)
		return;

	buddy = cell->data;
	if (acct->proto->get_profile != NULL)
		acct->proto->get_profile(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_refresh) {
	imwindow_blist_draw(cur_window());
}

USER_COMMAND(cmd_blist_remove) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	if (buddy_remove(acct, buddy->nname, 1) != 0) {
		screen_err_msg("%s could not be removed from %s's buddy list",
			buddy->name, acct->username);
	} else {
		char *name = xstrdup(buddy->name);
		screen_cmd_output("%s has been removed from %s's buddy list",
			name, acct->username);
		free(name);
	}
}

USER_COMMAND(cmd_blist_remove_block) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	if (buddy_remove_block(acct, buddy->nname, 1) != 0) {
		screen_err_msg("%s is not on %s's blocked users list",
			buddy->name, acct->username);
	} else {
		screen_cmd_output("%s has been removed from %s's blocked users list",
			buddy->name, acct->username);
	}
}

USER_COMMAND(cmd_blist_remove_permit) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	if (buddy_remove_permit(acct, buddy->nname, 1) != 0) {
		screen_err_msg("%s is not on %s's permitted users list",
			buddy->name, acct->username);
	} else {
		screen_cmd_output("%s has been removed from %s's permited users list",
			buddy->name, acct->username);
	}
}

USER_COMMAND(cmd_blist_select) {
	struct slist_cell *cell;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL)
		return;

	if (cell->type == TYPE_LIST_CELL)
		cmd_blist_collapse(acct, NULL);
	else {
		struct buddy *buddy = cell->data;

		cmd_blist_goto(acct, buddy->nname);
	}
}

USER_COMMAND(cmd_blist_show) {
	imwindow_blist_show(cur_window());
}

USER_COMMAND(cmd_blist_start) {
	blist_cursor_start(acct->blist);
}

USER_COMMAND(cmd_blist_toggle) {
	imwindow_blist_toggle(cur_window());
}

USER_COMMAND(cmd_blist_toggle_focus) {
	struct imwindow *win = cur_window();

	imwindow_blist_toggle(win);
	imwindow_switch_focus(win);
	imwindow_blist_draw(win);
}

USER_COMMAND(cmd_blist_up) {
	blist_cursor_up(acct->blist);
}

USER_COMMAND(cmd_blist_warn) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	pork_send_warn(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_warn_anon) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	pork_send_warn_anon(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_width) {
	u_int32_t new_len;
	struct blist *blist = acct->blist;

	if (args == NULL)
		return;

	if (str_to_uint(args, &new_len) != 0) {
		screen_err_msg("Error: invalid width: %s", args);
		return;
	}

	screen_blist_width(blist, new_len);
}

static struct command blist_command[] = {
	{ "add_block",		cmd_blist_add_block		},
	{ "add_permit",		cmd_blist_add_permit	},
	{ "away",			cmd_blist_away			},
	{ "collapse",		cmd_blist_collapse		},
	{ "down",			cmd_blist_down			},
	{ "end",			cmd_blist_end			},
	{ "goto",			cmd_blist_goto			},
	{ "hide",			cmd_blist_hide			},
	{ "page_down",		cmd_blist_pgdown		},
	{ "page_up",		cmd_blist_pgup			},
	{ "profile",		cmd_blist_profile		},
	{ "refresh",		cmd_blist_refresh		},
	{ "remove",			cmd_blist_remove		},
	{ "remove_block",	cmd_blist_remove_block	},
	{ "remove_permit",	cmd_blist_remove_permit	},
	{ "select",			cmd_blist_select		},
	{ "show",			cmd_blist_show			},
	{ "start",			cmd_blist_start			},
	{ "toggle",			cmd_blist_toggle		},
	{ "toggle_focus",	cmd_blist_toggle_focus	},
	{ "up",				cmd_blist_up			},
	{ "warn",			cmd_blist_warn			},
	{ "warn_anon",		cmd_blist_warn_anon		},
	{ "width",			cmd_blist_width			},
};

struct command_set blist_set = { blist_command, array_elem(blist_command), "blist " };
