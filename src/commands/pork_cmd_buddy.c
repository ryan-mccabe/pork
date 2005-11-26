/*
** pork_buddy.c - /buddy commands.
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
#include <pork_misc.h>
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
#include <pork_buddy.h>
#include <pork_msg.h>
#include <pork_command.h>

USER_COMMAND(cmd_buddy_add) {
	char *screen_name;
	char *group_name;
	struct bgroup *group;
	struct buddy *buddy;

	screen_name = strsep(&args, " ");
	if (screen_name == NULL || blank_str(screen_name)) {
		screen_err_msg("syntax is /buddy add <user> <group>");
		return;
	}

	buddy = buddy_find(acct, screen_name);
	if (buddy != NULL) {
		screen_err_msg("%s is already a member of the group %s",
			screen_name, buddy->group->name);
		return;
	}

	group_name = args;
	if (group_name == NULL || blank_str(group_name)) {
		screen_err_msg("syntax is /buddy add <user> <group>");
		return;
	}

	group = group_find(acct, group_name);
	if (group == NULL) {
		screen_err_msg("The group %s does not exist on %s's buddy list",
			group_name, acct->username);
		return;
	}

	buddy_add(acct, screen_name, group, 1);
}

USER_COMMAND(cmd_buddy_block) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_add_block(acct, args, 1) == -1) {
		screen_err_msg("%s is already on %s's blocked users list",
			acct->username, args);
	} else {
		screen_cmd_output("%s has been added %s's to blocked users list",
			acct->username, args);
	}
}

USER_COMMAND(cmd_buddy_permit) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_add_permit(acct, args, 1) == -1) {
		screen_err_msg("%s is already on %s's permitted users list",
			acct->username, args);
	} else {
		screen_cmd_output("%s has been added to %s's permitted users list",
			args, acct->username);
	}
}

USER_COMMAND(cmd_buddy_add_group) {
	if (args == NULL || blank_str(args))
		return;

	group_add(acct, args);
}

USER_COMMAND(cmd_buddy_alias) {
	struct buddy *buddy;
	char *buddy_name;
	char *alias;

	buddy_name = strsep(&args, " ");
	if (buddy_name == NULL)
		return;

	alias = args;
	if (alias == NULL || blank_str(alias))
		return;

	buddy = buddy_find(acct, buddy_name);
	if (buddy == NULL) {
		screen_err_msg("%s is not on %s's buddy list",
			acct->username, buddy_name);
		return;
	}

	if (buddy_alias(acct, buddy, alias, 1) != -1) {
		struct imwindow *win;

		screen_cmd_output("%s is now known as %s to %s",
			buddy_name, alias, acct->username);

		/*
		** If there's a conversation window with this user open, we
		** should change its title.
		*/

		win = imwindow_find(acct, buddy->nname);
		if (win != NULL)
			imwindow_rename(win, buddy->name);
	}
}

USER_COMMAND(cmd_buddy_awaymsg) {
	struct imwindow *win = cur_window();

	if (acct->proto->get_away_msg == NULL)
		return;

	if (args == NULL || blank_str(args)) {
		if (win->type == WIN_TYPE_PRIVMSG)
			args = win->target;
		else
			args = acct->username;
	}

	acct->proto->get_away_msg(acct, args);
}

USER_COMMAND(cmd_buddy_clear_block) {
	buddy_clear_block(acct);
}

USER_COMMAND(cmd_buddy_clear_permit) {
	buddy_clear_permit(acct);
}

USER_COMMAND(cmd_buddy_list) {
	struct imwindow *win = cur_window();
	struct buddy_pref *pref = acct->buddy_pref;
	dlist_t *gcur;

	gcur = pref->group_list;
	if (gcur != NULL) {
		screen_win_msg(win, 0, 1, 1, MSG_TYPE_CMD_OUTPUT,
			"%s's buddy list: ", acct->username);
	}

	while (gcur != NULL) {
		struct bgroup *gr = gcur->data;
		dlist_t *bcur = gr->member_list;

		screen_win_msg(win, 0, 0, 1, MSG_TYPE_CMD_OUTPUT, "+ %s (%u/%u)",
			gr->name, gr->num_online, gr->num_members);

		while (bcur != NULL) {
			struct buddy *buddy = bcur->data;

			if (buddy->status != STATUS_OFFLINE) {
				char buddy_status[128];
				size_t i = 0;
				size_t len = sizeof(buddy_status);
				char *color_attr = "%G";

				buddy_status[0] = '\0';

				if (buddy->idle_time > 0) {
					char time_buf[64];
					int ret;

					color_attr = "%Y";

					time_to_str(buddy->idle_time, time_buf, sizeof(time_buf));

					ret = snprintf(buddy_status, len, "idle: %s ", time_buf);
					if (ret < 0 || (size_t) ret >= len) {
						screen_err_msg("Output was too long to display");
						return;
					}

					i += ret;
					len -= ret;
				}

				if (buddy->warn_level > 0) {
					snprintf(&buddy_status[i], len, "warn level: %d%%",
						buddy->warn_level);
				}

				screen_win_msg(win, 0, 0, 1, MSG_TYPE_CMD_OUTPUT,
					" %so%%x %s %s", color_attr, buddy->name, buddy_status);
			} else {
				screen_win_msg(win, 0, 0, 1, MSG_TYPE_CMD_OUTPUT,
					" %%Ro%%x %s", buddy->name);
			}

			bcur = bcur->next;
		}

		gcur = gcur->next;
	}
}

USER_COMMAND(cmd_buddy_list_permit) {
	struct buddy_pref *pref = acct->buddy_pref;
	dlist_t *cur;

	cur = pref->permit_list;
	if (cur == NULL) {
		screen_cmd_output("%s's permitted users list is empty", acct->username);
		return;
	}

	screen_cmd_output("%s's permitted users list:", acct->username);

	while (cur != NULL) {
		screen_cmd_output(" - %s", (char *) cur->data);
		cur = cur->next;
	}
}

USER_COMMAND(cmd_buddy_list_block) {
	struct buddy_pref *pref = acct->buddy_pref;
	dlist_t *cur;

	cur = pref->block_list;
	if (cur == NULL) {
		screen_cmd_output("%s's blocked users list is empty", acct->username);
		return;
	}

	screen_cmd_output("%s's blocked users list:", acct->username);

	while (cur != NULL) {
		screen_cmd_output(" - %s", (char *) cur->data);
		cur = cur->next;
	}
}

USER_COMMAND(cmd_buddy_profile) {
	if (acct->proto->get_profile == NULL)
		return;

	if (args == NULL || blank_str(args)) {
		struct imwindow *win = cur_window();

		if (win->type == WIN_TYPE_PRIVMSG)
			args = win->target;
		else
			args = acct->username;
	}

	acct->proto->get_profile(acct, args);
}

USER_COMMAND(cmd_buddy_remove_permit) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_remove_permit(acct, args, 1) != 0) {
		screen_err_msg("%s is not on %s's permitted users list",
			args, acct->username);
	} else {
		screen_cmd_output("%s has been removed from %s's permited users list",
			args, acct->username);
	}
}

USER_COMMAND(cmd_buddy_unblock) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_remove_block(acct, args, 1) != 0) {
		screen_err_msg("%s is not on %s's blocked users list",
			args, acct->username);
	} else {
		screen_cmd_output("%s has been removed from %s's blocked users list",
			args, acct->username);
	}
}

USER_COMMAND(cmd_buddy_remove) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_remove(acct, args, 1) != 0)
		screen_err_msg("%s is not on %s's buddy list", args, acct->username);
	else {
		screen_cmd_output("%s has been removed from %s's buddy list",
			args, acct->username);
	}
}

USER_COMMAND(cmd_buddy_remove_group) {
	if (args == NULL || blank_str(args))
		return;

	if (group_remove(acct, args, 1) != 0) {
		screen_err_msg("The group %s does not exist on %s's buddy list",
			args, acct->username);
	} else {
		screen_cmd_output("The group %s has been removed from %s's buddy list",
			args, acct->username);
	}
}

USER_COMMAND(cmd_buddy_seen) {
	struct buddy *buddy;

	if (args == NULL || blank_str(args))
		return;

	buddy = buddy_find(acct, args);
	if (buddy == NULL) {
		screen_err_msg("%s is not on %s's buddy list", args, acct->username);
		return;
	}

	if (buddy->status != STATUS_OFFLINE) {
		screen_cmd_output("%s is currently online", buddy->name);
		return;
	}

	if (!buddy->last_seen) {
		screen_cmd_output("%s has never been seen online by %s",
			args, acct->username);
	} else {
		char *p;
		char timebuf[64];
		char timestr[64];
		u_int32_t time_diff = (u_int32_t) time(NULL) - buddy->last_seen;

		time_to_str(time_diff / 60 , timebuf, sizeof(timebuf));

		xstrncpy(timestr,
			asctime(localtime((time_t *) &buddy->last_seen)), sizeof(timestr));

		p = strchr(timestr, '\n');
		if (p != NULL)
			*p = '\0';

		screen_cmd_output("%s last saw %s online %s (%s ago)",
			acct->username, buddy->name, timestr, timebuf);
	}
}

USER_COMMAND(cmd_buddy_warn) {
	if (acct->proto->warn == NULL)
		return;

	if (args == NULL || blank_str(args)) {
		struct imwindow *win = cur_window();

		if (win->type == WIN_TYPE_PRIVMSG)
			args = win->target;
		else
			return;
	}

	pork_send_warn(acct, args);
}

USER_COMMAND(cmd_buddy_warn_anon) {
	if (acct->proto->warn_anon == NULL)
		return;

	if (args == NULL || blank_str(args)) {
		struct imwindow *win = cur_window();

		if (win->type == WIN_TYPE_PRIVMSG)
			args = win->target;
		else
			return;
	}

	pork_send_warn_anon(acct, args);
}

static struct command buddy_command[] = {
	{ "add",			cmd_buddy_add			},
	{ "add_group",		cmd_buddy_add_group		},
	{ "alias",			cmd_buddy_alias			},
	{ "awaymsg",		cmd_buddy_awaymsg		},
	{ "block",			cmd_buddy_block			},
	{ "clear_block",	cmd_buddy_clear_block	},
	{ "clear_permit",	cmd_buddy_clear_permit	},
	{ "list",			cmd_buddy_list			},
	{ "list_block",		cmd_buddy_list_block	},
	{ "list_permit",	cmd_buddy_list_permit	},
	{ "permit",			cmd_buddy_permit		},
	{ "profile",		cmd_buddy_profile		},
	{ "remove",			cmd_buddy_remove		},
	{ "remove_group",	cmd_buddy_remove_group	},
	{ "remove_permit",	cmd_buddy_remove_permit	},
	{ "seen",			cmd_buddy_seen			},
	{ "unblock",		cmd_buddy_unblock		},
	{ "warn",			cmd_buddy_warn			},
	{ "warn_anon",		cmd_buddy_warn_anon		},
};

struct command_set buddy_set = { buddy_command, array_elem(buddy_command), "buddy " };
