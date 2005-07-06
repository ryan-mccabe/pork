/*
** pork_command.c - interface to commands typed by the user
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
** Copyright (C) 2002-2004 Amber Adams <amber@ojnk.net>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_buddy.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_imsg.h>
#include <pork_imwindow.h>
#include <pork_imwindow_set.h>
#include <pork_input_set.h>
#include <pork_buddy_list.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_acct_set.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_html.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_events.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_alias.h>
#include <pork_chat.h>
#include <pork_conf.h>
#include <pork_perl.h>
#include <pork_timer.h>
#include <pork_transfer.h>
#include <pork_msg.h>
#include <pork_command.h>
#include <pork_command_defs.h>
#include <pork_help.h>

extern struct sockaddr_storage local_addr;
extern in_port_t local_port;

static void print_binding(void *data, void *nothing);
static void print_alias(void *data, void *nothing);
static int cmd_compare(const void *l, const void *r);
static void print_timer(void *data, void *nothing);
static int run_one_command(struct pork_acct *, char *str, u_int32_t set);

/*
** Note that the "struct command" arrays are arranged in alphabetical
** order. They have to be like that.
*/

/*
** The / command set.
*/

static struct command command[] = {
	{ "",			cmd_send			},
	{ "acct",		cmd_acct			},
	{ "alias",		cmd_alias			},
	{ "auto",		cmd_auto			},
	{ "away",		cmd_away			},
	{ "bind",		cmd_bind			},
	{ "blist",		cmd_blist			},
	{ "buddy",		cmd_buddy			},
	{ "chat",		cmd_chat			},
	{ "connect",	cmd_connect			},
	{ "disconnect", cmd_disconnect		},
	{ "echo",		cmd_echo			},
	{ "event",		cmd_event			},
	{ "file",		cmd_file			},
	{ "help",		cmd_help			},
	{ "history",	cmd_history			},
	{ "idle",		cmd_idle			},
	{ "input",		cmd_input			},
	{ "laddr",		cmd_laddr			},
	{ "lastlog",	cmd_lastlog			},
	{ "load",		cmd_load			},
	{ "lport",		cmd_lport			},
	{ "me",			cmd_me				},
	{ "msg",		cmd_msg				},
	{ "nick",		cmd_nick			},
	{ "notice",		cmd_notice			},
	{ "perl",		cmd_perl			},
	{ "perl_dump",	cmd_perl_dump		},
	{ "perl_load",	cmd_perl_load		},
	{ "ping",		cmd_ping			},
	{ "profile",	cmd_profile,		},
	{ "query",		cmd_query			},
	{ "quit",		cmd_quit			},
	{ "refresh",	cmd_refresh			},
	{ "save",		cmd_save			},
	{ "scroll",		cmd_scroll			},
	{ "set",		cmd_set				},
	{ "timer",		cmd_timer			},
	{ "unalias",	cmd_unalias			},
	{ "unbind",		cmd_unbind			},
	{ "win",		cmd_win				},
};

/*
** /input commands
*/

static struct command input_command[] = {
	{ "backspace",				cmd_input_bkspace			},
	{ "clear",					cmd_input_clear				},
	{ "clear_next_word",		cmd_input_clear_next		},
	{ "clear_prev_word",		cmd_input_clear_prev		},
	{ "clear_to_end",			cmd_input_clear_to_end		},
	{ "clear_to_start",			cmd_input_clear_to_start	},
	{ "delete",					cmd_input_delete			},
	{ "end",					cmd_input_end				},
	{ "find_next_cmd",			cmd_input_find_next_cmd		},
	{ "focus_next",				cmd_input_focus_next		},
	{ "insert",					cmd_input_insert			},
	{ "left",					cmd_input_left				},
	{ "next_word",				cmd_input_next_word			},
	{ "prev_word",				cmd_input_prev_word			},
	{ "prompt",					cmd_input_prompt			},
	{ "remove",					cmd_input_remove			},
	{ "right",					cmd_input_right				},
	{ "send",					cmd_input_send				},
	{ "set",					cmd_input_set				},
	{ "start",					cmd_input_start				},
};

USER_COMMAND(cmd_input_bkspace) {
	input_bkspace(cur_window()->input);
}

USER_COMMAND(cmd_input_clear) {
	input_clear_line(cur_window()->input);
}

USER_COMMAND(cmd_input_clear_prev) {
	input_clear_prev_word(cur_window()->input);
}

USER_COMMAND(cmd_input_clear_next) {
	input_clear_next_word(cur_window()->input);
}

USER_COMMAND(cmd_input_clear_to_end) {
	input_clear_to_end(cur_window()->input);
}

USER_COMMAND(cmd_input_focus_next) {
	struct imwindow *win = cur_window();

	imwindow_switch_focus(win);
	imwindow_blist_draw(win);
}

USER_COMMAND(cmd_input_clear_to_start) {
	input_clear_to_start(cur_window()->input);
}

USER_COMMAND(cmd_input_delete) {
	input_delete(cur_window()->input);
}

USER_COMMAND(cmd_input_end) {
	input_end(cur_window()->input);
}

USER_COMMAND(cmd_input_insert) {
	if (args != NULL)
		input_insert_str(cur_window()->input, args);
}

USER_COMMAND(cmd_input_left) {
	input_move_left(cur_window()->input);
}

USER_COMMAND(cmd_input_prev_word) {
	input_prev_word(cur_window()->input);
}

USER_COMMAND(cmd_input_prompt) {
	if (args == NULL || blank_str(args) ||
		!strcasecmp(args, "off") || !strcasecmp(args, "false"))
	{
		input_set_prompt(cur_window()->input, NULL);
		screen_cmd_output("Input prompt set to off");
	} else if (	args != NULL &&
				(!strcasecmp(args, "on") || !strcasecmp(args, "true")))
	{
		char *prompt;

		prompt = opt_get_str(cur_window()->input->prefs, INPUT_OPT_PROMPT_STR);
		if (prompt != NULL)
			input_set_prompt(cur_window()->input, prompt);
		else {
			screen_err_msg("No prompt format has been specified for this input");
		}
	}
}

USER_COMMAND(cmd_input_next_word) {
	input_next_word(cur_window()->input);
}

USER_COMMAND(cmd_input_remove) {
	if (args != NULL && !blank_str(args)) {
		int len;

		if (str_to_int(args, &len) == -1)
			screen_err_msg("Bad number of characters: %s", args);
		else
			input_remove(cur_window()->input, len);
	}
}

USER_COMMAND(cmd_input_right) {
	input_move_right(cur_window()->input);
}

USER_COMMAND(cmd_input_send) {
	struct imwindow *imwindow = cur_window();
	struct pork_input *input = imwindow->input;
	static int recursion;

	/*
	** This is kind of a hack, but it's necessary if the client
	** isn't to crash when someone types "/input send"
	*/

	if (recursion == 1 && args == NULL)
		return;

	recursion = 1;

	if (args != NULL)
		input_set_buf(input, args);

	if (input->len > 0) {
		char *input_str = xstrdup(input_get_buf_str(input));

		if (args == NULL)
			input_history_add(input);

		input_clear_line(input);

		if (event_generate(acct->events, EVENT_SEND_LINE,
				input_str, acct->refnum))
		{
			goto out;
		}

		if (input_str[0] == opt_get_char(screen.global_prefs, OPT_CMDCHARS))
			run_command(acct, &input_str[1]);
		else
			cmd_send(acct, input_str);
out:
		free(input_str);
	}

	recursion = 0;
}

USER_COMMAND(cmd_input_set) {
	struct pork_input *input;
	struct pref_val *pref;

	if (args == NULL || blank_str(args)) {
		input = cur_window()->input;
		pref = input->prefs;
	} else if (!strncasecmp(args, "-default", 8)) {
		args += 8;
		while (args[0] == ' ')
			args++;

		input = NULL;
		pref = input_get_default_prefs();
	} else {
		input = cur_window()->input;
		pref = input->prefs;
	}

	opt_set_var(pref, args, input);
}

USER_COMMAND(cmd_input_start) {
	input_home(cur_window()->input);
}

/*
** /scroll commands
*/

static struct command scroll_command[] = {
	{ "by",				cmd_scroll_by			},
	{ "down",			cmd_scroll_down			},
	{ "end",			cmd_scroll_end			},
	{ "page_down",		cmd_scroll_pgdown		},
	{ "page_up",		cmd_scroll_pgup			},
	{ "start",			cmd_scroll_start		},
	{ "up",				cmd_scroll_up			},
};

USER_COMMAND(cmd_scroll_by) {
	int lines;

	if (args == NULL)
		return;

	if (str_to_int(args, &lines) != 0) {
		screen_err_msg("Invalid number of lines: %s", args);
		return;
	}

	imwindow_scroll_by(cur_window(), lines);
}

USER_COMMAND(cmd_scroll_down) {
	imwindow_scroll_down(cur_window());
}

USER_COMMAND(cmd_scroll_end) {
	imwindow_scroll_end(cur_window());
}

USER_COMMAND(cmd_scroll_pgdown) {
	imwindow_scroll_page_down(cur_window());
}

USER_COMMAND(cmd_scroll_pgup) {
	imwindow_scroll_page_up(cur_window());
}

USER_COMMAND(cmd_scroll_start) {
	imwindow_scroll_start(cur_window());
}

USER_COMMAND(cmd_scroll_up) {
	imwindow_scroll_up(cur_window());
}

/*
** /file commands.
*/

static struct command file_command[] = {
	{ "abort",				cmd_file_cancel		},
	{ "cancel",				cmd_file_cancel 	},
	{ "get",				cmd_file_get		},
	{ "list",				cmd_file_list		},
	{ "resume",				cmd_file_resume		},
	{ "send",				cmd_file_send		},
};

USER_COMMAND(cmd_file_cancel) {
	u_int32_t refnum;
	struct file_transfer *xfer;
	char *refnum_str;

	if (args == NULL || blank_str(args))
		return;

	refnum_str = strsep(&args, " ");
	if (refnum_str == NULL)
		return;

	if (!strcasecmp(refnum_str, "all")) {
		transfer_abort_all(acct, TRANSFER_DIR_ANY);
		return;
	}

	if (!strcasecmp(refnum_str, "send")) {
		transfer_abort_all(acct, TRANSFER_DIR_SEND);
		return;
	}

	if (!strcasecmp(refnum_str, "recv") || !strcasecmp(refnum_str, "receive")) {
		transfer_abort_all(acct, TRANSFER_DIR_RECV);
		return;
	}

	if (str_to_uint(args, &refnum) != 0) {
		screen_err_msg("Invalid file transfer refnum: %s", args);
		return;
	}

	xfer = transfer_find_refnum(acct, refnum);
	if (xfer == NULL) {
		screen_err_msg("Invalid file transfer refnum: %s", args);
		return;
	}

	if (transfer_cancel_local(xfer) != 0)
		screen_err_msg("Error canceling file transfer %s", args);
}

USER_COMMAND(cmd_file_list) {
	if (acct->transfer_list != NULL) {
		dlist_t *cur = acct->transfer_list;

		while (cur != NULL) {
			struct file_transfer *xfer = cur->data;

			/* XXX - make this a format str */
			screen_cmd_output("%u: %s %s [%s: %u/%u (%.02f%%) - %.04f KB/s]",
				xfer->refnum,
				xfer->peer_username, xfer->fname_local,
				transfer_status_str(xfer),
				(u_int32_t) xfer->bytes_sent, (u_int32_t) xfer->file_len,
				(float) xfer->bytes_sent / xfer->file_len * 100,
				transfer_avg_speed(xfer));

			cur = cur->next;
		}
	}
}

USER_COMMAND(cmd_file_get) {
	char *refnum_str;
	u_int32_t refnum;
	struct file_transfer *xfer;

	if (args == NULL || blank_str(args))
		return;

	refnum_str = strsep(&args, " ");
	if (refnum_str == NULL)
		return;

	if (!strcasecmp(refnum_str, "all")) {
		transfer_get_all(acct);
		return;
	}

	if (str_to_uint(refnum_str, &refnum) != 0) {
		screen_err_msg("Invalid file transfer refnum: %s", refnum_str);
		return;
	}

	xfer = transfer_find_refnum(acct, refnum);
	if (xfer == NULL) {
		screen_err_msg("Invalid file transfer refnum: %s", refnum_str);
		return;
	}

	if (args != NULL && blank_str(args))
		args = NULL;

	transfer_get(xfer, args);
}

USER_COMMAND(cmd_file_resume) {
}

USER_COMMAND(cmd_file_send) {
	char *dest;

	if (args == NULL)
		return;

	dest = strsep(&args, " ");
	if (dest == NULL || args == NULL) {
		screen_err_msg("You must specify a user and a filename");
		return;
	}

	transfer_send(acct, dest, args);
}

/*
** /win commands.
*/

static struct command window_command[] = {
	{ "bind",				cmd_win_bind		},
	{ "bind_next",			cmd_win_bind_next	},
	{ "clear",				cmd_win_clear		},
	{ "close",				cmd_win_close		},
	{ "dump",				cmd_win_dump		},
	{ "erase",				cmd_win_erase		},
	{ "ignore",				cmd_win_ignore		},
	{ "list",				cmd_win_list		},
	{ "next",				cmd_win_next		},
	{ "prev",				cmd_win_prev		},
	{ "rename",				cmd_win_rename		},
	{ "renumber",			cmd_win_renumber	},
	{ "set",				cmd_win_set			},
	{ "skip",				cmd_win_skip		},
	{ "swap",				cmd_win_swap		},
	{ "unignore",			cmd_win_unignore	},
	{ "unskip",				cmd_win_unskip		},
};

USER_COMMAND(cmd_win_bind) {
	struct imwindow *imwindow = cur_window();
	u_int32_t refnum;
	int ret;

	if (args == NULL || blank_str(args)) {
		if (imwindow->owner != NULL && imwindow->owner->username != NULL) {
			screen_cmd_output("This window is bound to account %s [refnum %u]",
				imwindow->owner->username, imwindow->owner->refnum);
		} else
			screen_cmd_output("This window is bound to no account");

		return;
	}

	if (str_to_uint(args, &refnum) == -1) {
		screen_err_msg("Bad account refnum: %s", args);
		return;
	}

	ret = imwindow_bind_acct(imwindow, refnum);
	if (ret == -1) {
		if (imwindow->type == WIN_TYPE_CHAT)
			screen_err_msg("You can't rebind chat windows");
		else
			screen_err_msg("Account %s isn't signed on", args);
	} else {
		screen_cmd_output("This window is now bound to account %s [refnum %u]",
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
	if (args == NULL || blank_str(args)) {
		screen_err_msg("No output file specified");
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

	if (args != NULL && !blank_str(args)) {
		u_int32_t refnum;

		if (str_to_uint(args, &refnum) != 0) {
			screen_err_msg("Bad window refnum: %s", args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg("No window with refnum %u", refnum);
			return;
		}
	} else
		win = cur_window();

	imwindow_ignore(win);
}

USER_COMMAND(cmd_win_list) {
	dlist_t *cur;
	static const char *win_types[] = { "Status", "Conv", "Chat" };

	screen_cmd_output("Window List:");

	screen_cmd_output("REFNUM\t\tNAME\t\tTYPE\t\tTARGET");
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

USER_COMMAND(cmd_win_prev) {
	screen_cycle_bak();
}

USER_COMMAND(cmd_win_rename) {
	struct imwindow *win = cur_window();

	if (args == NULL)
		screen_cmd_output("Window %u has name \"%s\"", win->refnum, win->name);
	else
		imwindow_rename(win, args);
}

USER_COMMAND(cmd_win_renumber) {
	u_int32_t num;

	if (args == NULL || blank_str(args)) {
		screen_cmd_output("This is window %u", cur_window()->refnum);
		return;
	}

	if (str_to_uint(args, &num) != 0) {
		screen_err_msg("Bad window number: %s", args);
		return;
	}

	screen_renumber(cur_window(), num);
}

USER_COMMAND(cmd_win_set) {
	struct imwindow *win;
	struct pref_val *pref;

	if (args == NULL || blank_str(args)) {
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
			screen_err_msg("Invalid window refnum: %s", args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg("No window with refnum %u exists", refnum);
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

	if (args != NULL && !blank_str(args)) {
		u_int32_t refnum;

		if (str_to_uint(args, &refnum) != 0) {
			screen_err_msg("Bad window refnum: %s", args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg("No window with refnum %u", refnum);
			return;
		}
	} else
		win = cur_window();

	imwindow_skip(win);
}

USER_COMMAND(cmd_win_swap) {
	u_int32_t num;

	if (args == NULL || blank_str(args))
		return;

	if (str_to_uint(args, &num) != 0) {
		screen_err_msg("Invalid window refnum: %s", args);
		return;
	}

	if (screen_goto_window(num) != 0)
		screen_err_msg("No such window: %s", args);
}

USER_COMMAND(cmd_win_unignore) {
	struct imwindow *win;

	if (args != NULL && !blank_str(args)) {
		u_int32_t refnum;

		if (str_to_uint(args, &refnum) != 0) {
			screen_err_msg("Bad window refnum: %s", args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg("No window with refnum %u", refnum);
			return;
		}
	} else
		win = cur_window();

	imwindow_unignore(win);
}

USER_COMMAND(cmd_win_unskip) {
	struct imwindow *win;

	if (args != NULL && !blank_str(args)) {
		u_int32_t refnum;

		if (str_to_uint(args, &refnum) != 0) {
			screen_err_msg("Bad window refnum: %s", args);
			return;
		}

		win = imwindow_find_refnum(refnum);
		if (win == NULL) {
			screen_err_msg("No window with refnum %u", refnum);
			return;
		}
	} else
		win = cur_window();

	imwindow_unskip(win);
}

/*
** History Manipulation.
*/

static struct command history_command[] = {
	{ "clear",		cmd_history_clear	},
	{ "list",		cmd_history_list	},
	{ "next",		cmd_history_next	},
	{ "prev",		cmd_history_prev	},
};

USER_COMMAND(cmd_history_clear) {
	input_history_clear(cur_window()->input);
}

USER_COMMAND(cmd_history_list) {
	struct imwindow *win = cur_window();
	struct pork_input *input = win->input;
	dlist_t *cur = input->history_end;
	u_int32_t i = 0;

	if (cur == NULL)
		return;

	screen_win_msg(win, 0, 0, 0, MSG_TYPE_CMD_OUTPUT, "Command history:");

	do {
		screen_win_msg(win, 0, 0, 0, MSG_TYPE_CMD_OUTPUT, "%u: %s", i++,
			(char *) cur->data);
		cur = cur->prev;
	} while (cur != NULL);
}

USER_COMMAND(cmd_history_next) {
	input_history_next(cur_window()->input);
}

USER_COMMAND(cmd_history_prev) {
	input_history_prev(cur_window()->input);
}

/*
** /buddy commands
*/

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
		screen_err_msg("The group %s does not exist", group_name);
		return;
	}

	buddy_add(acct, screen_name, group, 1);
}

USER_COMMAND(cmd_buddy_block) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_add_block(acct, args, 1) == -1)
		screen_err_msg("%s is already on your block list", args);
	else
		screen_cmd_output("%s added to block list", args);
}

USER_COMMAND(cmd_buddy_permit) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_add_permit(acct, args, 1) == -1)
		screen_err_msg("%s is already on your permit list", args);
	else
		screen_cmd_output("%s added to permit list", args);
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
	struct imwindow *win;

	buddy_name = strsep(&args, " ");
	if (buddy_name == NULL)
		return;

	alias = args;
	if (alias == NULL || blank_str(alias))
		return;

	buddy = buddy_find(acct, buddy_name);
	if (buddy == NULL) {
		screen_err_msg("%s is not on your buddy list", buddy_name);
		return;
	}

	screen_cmd_output("alias: %s -> %s", buddy_name, alias);
	buddy_alias(acct, buddy, alias, 1);

	/*
	** If there's a conversation window with this user open, we
	** should change its title.
	*/

	win = imwindow_find(acct, buddy->nname);
	if (win != NULL)
		imwindow_rename(win, buddy->name);
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

	if (buddy_remove_permit(acct, args, 1) != 0)
		screen_err_msg("%s isn't on %s's permit list", args, acct->username);
	else {
		screen_cmd_output("%s removed from %s's permit list",
			args, acct->username);
	}
}

USER_COMMAND(cmd_buddy_unblock) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_remove_block(acct, args, 1) != 0)
		screen_err_msg("%s isn't on %s's block list", args, acct->username);
	else {
		screen_cmd_output("%s removed from %s's block list",
			args, acct->username);
	}
}

USER_COMMAND(cmd_buddy_remove) {
	if (args == NULL || blank_str(args))
		return;

	if (buddy_remove(acct, args, 1) != 0)
		screen_err_msg("%s isn't on %s's buddy list", args, acct->username);
	else {
		screen_cmd_output("%s removed from %s's buddy list",
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
		screen_cmd_output("The group %s was removed from %s's buddy list",
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

/*
** /blist commands
*/

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
	{ "up",				cmd_blist_up			},
	{ "warn",			cmd_blist_warn			},
	{ "warn_anon",		cmd_blist_warn_anon		},
	{ "width",			cmd_blist_width			},
};

USER_COMMAND(cmd_blist_add_block) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	cmd_buddy_block(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_add_permit) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	cmd_buddy_permit(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_away) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	cmd_buddy_awaymsg(acct, buddy->nname);
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
	if (args != NULL)
		cmd_query(acct, args);
	else {
		struct slist_cell *cell;
		struct buddy *buddy;
		struct blist *blist = acct->blist;

		cell = blist_get_cursor(blist);
		if (cell == NULL)
			return;

		if (cell->type != TYPE_LIST_CELL)
			return;

		buddy = cell->data;
		cmd_query(acct, buddy->nname);
	}
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
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	cmd_buddy_profile(acct, buddy->nname);
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
	cmd_buddy_remove(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_remove_block) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	cmd_buddy_unblock(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_remove_permit) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	cmd_buddy_remove_permit(acct, buddy->nname);
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
	cmd_buddy_warn(acct, buddy->nname);
}

USER_COMMAND(cmd_blist_warn_anon) {
	struct slist_cell *cell;
	struct buddy *buddy;
	struct blist *blist = acct->blist;

	cell = blist_get_cursor(blist);
	if (cell == NULL || cell->type == TYPE_LIST_CELL)
		return;

	buddy = cell->data;
	cmd_buddy_warn_anon(acct, buddy->nname);
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

/*
** /timer commands
*/

static struct command timer_command[] = {
	{ "add",			cmd_timer_add			},
	{ "del",			cmd_timer_del			},
	{ "del_refnum",		cmd_timer_del_refnum	},
	{ "list",			cmd_timer_list			},
	{ "purge",			cmd_timer_purge			},
};

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

/*
** /event commands
*/

static struct command event_command[] = {
	{ "add",			cmd_event_add			},
	{ "del",			cmd_event_del			},
	{ "del_refnum",		cmd_event_del_refnum	},
	{ "list",			cmd_event_list			},
	{ "purge",			cmd_event_purge			},
};

USER_COMMAND(cmd_event_add) {
	char *event_type;
	u_int32_t refnum;
	struct event *events = acct->events;

	if (args == NULL || *args == '\0') {
		event_list(events, NULL);
		return;
	}

	event_type = strsep(&args, " ");
	if (event_type == NULL) {
		event_list(events, NULL);
		return;
	}

	strtoupper(event_type);
	if (args == NULL) {
		event_list(events, event_type);
		return;
	}

	if (event_add(events, event_type, args, &refnum) != 0)
		screen_err_msg("Error adding handler for %s", event_type);
	else {
		screen_cmd_output("Event handler %s installed for %s (refnum %u)",
			args, event_type, refnum);
	}
}

USER_COMMAND(cmd_event_del) {
	char *event_type;
	int ret;

	if (args == NULL)
		return;

	event_type = strsep(&args, " ");
	if (event_type == NULL)
		return;

	ret = event_del_type(acct->events, event_type, args);
	if (ret == 0) {
		if (args == NULL) {
			screen_cmd_output("Successfully removed handler %s for %s",
				event_type, args);
		} else
			screen_cmd_output("Successfully removed all handlers for %s", args);
	} else {
		if (args == NULL) {
			screen_err_msg("Error removing handler %s for %s",
				event_type, args);
		} else
			screen_err_msg("Error removing all handlers for %s", args);
	}
}

USER_COMMAND(cmd_event_del_refnum) {
	u_int32_t refnum;

	if (args == NULL)
		return;

	if (str_to_uint(args, &refnum) != 0) {
		screen_err_msg("Invalid event refnum: %s", args);
		return;
	}

	if (event_del_refnum(acct->events, refnum) != 0)
		screen_err_msg("Error deleting event refnum %s", args);
	else
		screen_cmd_output("Event refnum %s was removed", args);
}

USER_COMMAND(cmd_event_list) {
	event_list(acct->events, args);
}

USER_COMMAND(cmd_event_purge) {
	event_purge(acct->events);
}

/*
** acct commands
*/

static struct command acct_command[] = {
	{ "list",	cmd_acct_list		},
	{ "save",	cmd_acct_save		},
	{ "set",	cmd_acct_set		},
};

USER_COMMAND(cmd_acct_list) {
	pork_acct_print_list();
}

USER_COMMAND(cmd_acct_save) {
	pork_acct_save(acct);
}

USER_COMMAND(cmd_acct_set) {
	struct pref_val *pref;

	if (args == NULL || blank_str(args)) {
		pref = acct->prefs;
	} else if (!strncasecmp(args, "-default", 8)) {
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
			screen_err_msg("Invalid account refnum: %s", args);
			return;
		}

		acct = pork_acct_get_data(refnum);
		if (acct == NULL) {
			screen_err_msg("No account with refnum %u exists", refnum);
			return;
		}

		pref = acct->prefs;		
	} else
		pref = acct->prefs;

	opt_set_var(pref, args, acct);
}

/*
** /chat commands
*/

static struct command chat_command[] = {
	{ "ban",				cmd_chat_ban			},
	{ "ignore",				cmd_chat_ignore			},
	{ "invite",				cmd_chat_invite			},
	{ "join",				cmd_chat_join			},
	{ "kick",				cmd_chat_kick			},
	{ "leave",				cmd_chat_leave			},
	{ "list",				cmd_chat_list			},
	{ "send",				cmd_chat_send			},
	{ "topic",				cmd_chat_topic			},
	{ "unignore",			cmd_chat_unignore		},
	{ "who",				cmd_chat_who			},
};

USER_COMMAND(cmd_chat_ban) {
	struct imwindow *win = cur_window();
	struct chatroom *chat;
	char *arg1;
	char *arg2;

	if (args == NULL)
		return;

	arg1 = strsep(&args, " ");

	chat = chat_find(acct, arg1);
	if (chat == NULL) {
		if (win->type == WIN_TYPE_CHAT && win->data != NULL)
			chat_ban(acct, win->data, arg1);
		else
			screen_err_msg("%s is not a member of %s", acct->username, arg1);

		return;
	}

	arg2 = strsep(&args, " ");
	if (arg2 != NULL)
		chat_ban(acct, chat, arg2);
}

USER_COMMAND(cmd_chat_ignore) {
	struct imwindow *imwindow = cur_window();
	char *chat_name;
	char *user_name;

	if (args == NULL)
		return;

	chat_name = strsep(&args, " ");
	user_name = args;

	if (user_name == NULL) {
		struct chatroom *chat = imwindow->data;

		if (imwindow->type != WIN_TYPE_CHAT || chat == NULL) {
			screen_err_msg("You must specify a chat room if the current window is not a chat window");
			return;
		}

		user_name = chat_name;
		chat_name = chat->title;
	}

	chat_ignore(acct, chat_name, user_name);
}

USER_COMMAND(cmd_chat_invite) {
	struct imwindow *imwindow = cur_window();
	char *chat_name;
	char *user_name;
	char *invite_msg;

	if (args == NULL)
		return;

	chat_name = strsep(&args, " ");
	user_name = strsep(&args, " ");
	invite_msg = args;

	if (user_name == NULL) {
		struct chatroom *chat = imwindow->data;

		if (imwindow->type != WIN_TYPE_CHAT || chat == NULL) {
			screen_err_msg("You must specify a chat room if the current window is not a chat window");
			return;
		}

		user_name = chat_name;
		chat_name = chat->title;
	}

	chat_invite(acct, chat_name, user_name, invite_msg);
}

USER_COMMAND(cmd_chat_join) {
	chat_join(acct, args);
}

USER_COMMAND(cmd_chat_kick) {
	struct imwindow *win = cur_window();
	struct chatroom *chat;
	char *arg1;
	char *arg2;

	if (args == NULL)
		return;

	arg1 = strsep(&args, " ");

	chat = chat_find(acct, arg1);
	if (chat == NULL) {
		if (win->type == WIN_TYPE_CHAT && win->data != NULL)
			chat_kick(acct, win->data, arg1, args);
		else
			screen_err_msg("%s is not a member of %s", acct->username, arg1);

		return;
	}

	arg2 = strsep(&args, " ");
	if (arg2 != NULL)
		chat_kick(acct, chat, arg2, args);
}

USER_COMMAND(cmd_chat_leave) {
	struct imwindow *win = cur_window();
	char *name = args;

	if (name == NULL || blank_str(name)) {
		struct chatroom *chat;

		if (win->type != WIN_TYPE_CHAT) {
			screen_err_msg("You must specify a chat room if the current window is not a chat window");
			return;
		}

		if (win->data == NULL)
			return;

		chat = win->data;
		name = chat->title;
	}

	chat_leave(acct, name, 1);
}

USER_COMMAND(cmd_chat_list) {
	chat_list(acct);
}

USER_COMMAND(cmd_chat_send) {
	struct imwindow *win;
	char *chat_name;

	if (args == NULL)
		return;

	chat_name = strsep(&args, " ");
	if (chat_name == NULL || args == NULL) {
		screen_err_msg("You must specify a chatroom and a message");
		return;
	}

	win = imwindow_find_chat_target(acct, chat_name);
	if (win == NULL || win->data == NULL) {
		screen_err_msg("%s is not joined to %s", acct->username, chat_name);
		return;
	}

	chat_send_msg(acct, win->data, chat_name, args);
}

USER_COMMAND(cmd_chat_topic) {
	struct imwindow *win = cur_window();
	char *topic = NULL;
	struct chatroom *chat = NULL;

	if (acct->proto->chat_set_topic == NULL)
		return;

	if (args != NULL) {
		topic = strchr(args, ' ');
		if (topic != NULL)
			*topic++ = '\0';

		chat = chat_find(acct, args);
	}

	if (chat == NULL) {
		if (topic != NULL)
			topic[-1] = ' ';

		topic = args;

		if (win->type == WIN_TYPE_CHAT)
			chat = win->data;
		else {
			screen_err_msg("You must specify a chat room if the current window isn't a chat window");
			return;
		}
	}

	acct->proto->chat_set_topic(acct, chat, topic);
}

USER_COMMAND(cmd_chat_unignore) {
	struct imwindow *imwindow = cur_window();
	char *chat_name;
	char *user_name;

	if (args == NULL)
		return;

	chat_name = strsep(&args, " ");
	user_name = args;

	if (user_name == NULL) {
		struct chatroom *chat = imwindow->data;

		if (imwindow->type != WIN_TYPE_CHAT || chat == NULL) {
			screen_err_msg("You must specify a chat room if the current window is not a chat window");
			return;
		}

		user_name = chat_name;
		chat_name = chat->title;
	}

	chat_unignore(acct, chat_name, user_name);
}

USER_COMMAND(cmd_chat_who) {
	struct imwindow *imwindow = cur_window();

	if (args == NULL || blank_str(args)) {
		struct chatroom *chat;

		if (imwindow->type != WIN_TYPE_CHAT) {
			screen_err_msg("You must specify a chat room if the current window is not a chat window");
			return;
		}

		chat = imwindow->data;
		args = chat->title;
	}

	chat_who(acct, args);
}

static struct command_set {
	struct command *set;
	size_t elem;
	char *type;
} command_set[] = {
	{	command,			array_elem(command),			"" 			},
	{	window_command,		array_elem(window_command),		"win "		},
	{	history_command,	array_elem(history_command),	"history "	},
	{	input_command,		array_elem(input_command),		"input "	},
	{	scroll_command,		array_elem(scroll_command),		"scroll "	},
	{	buddy_command,		array_elem(buddy_command),		"buddy "	},
	{	blist_command,		array_elem(blist_command),		"blist "	},
	{	timer_command,		array_elem(timer_command),		"timer "	},
	{	event_command,		array_elem(event_command),		"event "	},
	{	chat_command,		array_elem(chat_command),		"chat "		},
	{	file_command,		array_elem(file_command),		"file "		},
	{	acct_command,		array_elem(acct_command),		"acct "		},
};

/*
** Main command set.
*/

USER_COMMAND(cmd_alias) {
	char *alias;
	char *str;

	alias = strsep(&args, " ");
	if (alias == NULL || blank_str(alias)) {
		hash_iterate(&screen.alias_hash, print_alias, NULL);
		return;
	}

	str = args;
	if (str == NULL || blank_str(str)) {
		struct alias *lalias = alias_find(&screen.alias_hash, alias);

		if (lalias != NULL) {
			screen_cmd_output("%s is aliased to %s%s",
				lalias->alias, lalias->orig,
				(lalias->args != NULL ? lalias->args : ""));
		} else
			screen_err_msg("There is no alias for %s", alias);

		return;
	}

	if (alias_add(&screen.alias_hash, alias, str) == 0) {
		struct alias *lalias = alias_find(&screen.alias_hash, alias);

		if (lalias != NULL) {
			screen_cmd_output("%s is aliased to %s%s",
				lalias->alias, lalias->orig,
				(lalias->args != NULL ? lalias->args : ""));

			return;
		}
	}

	screen_err_msg("Error adding alias for %s", alias);
}

USER_COMMAND(cmd_auto) {
	char *target;

	if (args == NULL || !acct->connected)
		return;

	target = strsep(&args, " ");
	if (target == NULL || args == NULL)
		return;

	pork_msg_autoreply(acct, target, args);
}

USER_COMMAND(cmd_away) {
	if (args == NULL)
		pork_set_back(acct);
	else
		pork_set_away(acct, args);
}

USER_COMMAND(cmd_bind) {
	int key;
	char *func;
	char *key_str;
	struct key_binds *target_binds = cur_window()->active_binds;
	struct binding *binding;

	key_str = strsep(&args, " ");
	if (key_str == NULL || blank_str(key_str)) {
		hash_iterate(&target_binds->hash, print_binding, NULL);
		return;
	}

	if (key_str[0] == '-' && key_str[1] != '\0') {
		if (!strcasecmp(key_str, "-b") || !strcasecmp(key_str, "-buddy"))
			target_binds = &screen.binds.blist;
		else if (!strcasecmp(key_str, "-m") || !strcasecmp(key_str, "-main"))
			target_binds = &screen.binds.main;
		else {
			screen_err_msg("Bad bind flag: %s", key_str);
			return;
		}

		key_str = strsep(&args, " ");

		if (key_str == NULL || blank_str(key_str)) {
			hash_iterate(&target_binds->hash, print_binding, NULL);
			return;
		}
	}

	key = bind_get_keycode(key_str);
	if (key == -1) {
		screen_err_msg("Bad keycode: %s", key_str);
		return;
	}

	func = args;
	if (func != NULL) {
		if (*func == opt_get_char(screen.global_prefs, OPT_CMDCHARS) &&
			*(func + 1) != '\0')
		{
			func++;
		}

		if (blank_str(func))
			func = NULL;
	}

	if (func == NULL) {
		binding = bind_find(target_binds, key);
		if (binding != NULL)
			screen_cmd_output("%s is bound to %s", key_str, binding->binding);
		else
			screen_cmd_output("%s is not bound", key_str);

		return;
	}

	bind_add(target_binds, key, func);
	binding = bind_find(target_binds, key);
	if (binding != NULL) {
		screen_cmd_output("%s is bound to %s", key_str, binding->binding);
		return;
	}

	screen_err_msg("Error binding %s", key_str);
}

USER_COMMAND(cmd_connect) {
	int protocol = PROTO_AIM;
	char *user;

	if (args == NULL || blank_str(args))
		return;

	if (*args == '-') {
		char *p = strchr(++args, ' ');

		if (p != NULL)
			*p++ = '\0';

		protocol = proto_get_num(args);
		if (protocol == -1) {
			screen_err_msg("Invalid protocol: %s", args);
			return;
		}

		args = p;
	}

	user = strsep(&args, " ");
	pork_acct_connect(user, args, protocol);
}

USER_COMMAND(cmd_echo) {
	if (args != NULL)
		screen_win_msg(cur_window(), 0, 0, 1, MSG_TYPE_CMD_OUTPUT, args);
}

USER_COMMAND(cmd_disconnect) {
	u_int32_t dest;
	dlist_t *node;

	if (!acct->can_connect)
		return;

	if (args == NULL || blank_str(args))
		dest = acct->refnum;
	else {
		char *refnum = strsep(&args, " ");

		if (str_to_uint(refnum, &dest) == -1) {
			screen_err_msg("Bad account refnum: %s", refnum);
			return;
		}

		if (args != NULL && blank_str(args))
			args = NULL;
	}

	node = pork_acct_find(dest);
	if (node == NULL) {
		screen_err_msg("Account refnum %u is not logged in", dest);
		return;
	}

	acct = node->data;
	if (!acct->can_connect) {
		screen_err_msg("You cannot sign %s off", acct->username);
		return;
	}

	pork_acct_del(node, args);

	if (screen.status_win->owner == screen.null_acct)
		imwindow_bind_next_acct(screen.status_win);
}

USER_COMMAND(cmd_help) {
	char *section;

	if (args == NULL) {
		char buf[8192];

		if (pork_help_get_cmds("main", buf, sizeof(buf)) != -1) {
			screen_cmd_output("Help for the following commands is available:");
			screen_win_msg(cur_window(), 0, 0, 1, MSG_TYPE_CMD_OUTPUT,
				"\t%s", buf);
		} else
			screen_err_msg("Error: Can't find the help files");

		return;
	}

	section = strsep(&args, " ");
	if (section == NULL) {
		screen_err_msg("Error: Can't find the help files");
		return;
	}

	if (args == NULL) {
		char buf[8192];

		if (pork_help_print("main", section) == -1) {
			screen_err_msg("Help: Error: No such command or section: %s",
				section);
		} else {
			struct imwindow *win = cur_window();

			if (pork_help_get_cmds(section, buf, sizeof(buf)) != -1) {
				screen_win_msg(win, 0, 0, 1, MSG_TYPE_CMD_OUTPUT, " ");
				strtoupper(section);
				screen_win_msg(win, 0, 0, 1, MSG_TYPE_CMD_OUTPUT,
					"%%W%s COMMANDS", section);
				screen_win_msg(win, 0, 0, 1, MSG_TYPE_CMD_OUTPUT, "\t%s", buf);
				screen_cmd_output("Type /help %s <command> for the help text for a particular %s command.", section, section);
			}
		}
	} else {
		if (pork_help_print(section, args) == -1) {
			screen_err_msg("Help: Error: No such command in section %s",
				section);
		}
	}
}

USER_COMMAND(cmd_idle) {
	u_int32_t idle_secs = 0;

	if (args != NULL && !blank_str(args)) {
		if (str_to_uint(args, &idle_secs) != 0) {
			screen_err_msg("Invalid time specification: %s", args);
			return;
		}
	}

	pork_set_idle_time(acct, idle_secs);
}

USER_COMMAND(cmd_laddr) {
	if (args == NULL) {
		char buf[2048];

		if (get_hostname(&local_addr, buf, sizeof(buf)) != 0)
			xstrncpy(buf, "0.0.0.0", sizeof(buf));

		screen_cmd_output("New connections will use the local address %s", buf);
		return;
	}

	if (get_addr(args, &local_addr) != 0) {
		screen_err_msg("Invalid local address: %s", args);
		return;
	}

	screen_cmd_output("New connections will use the local address %s", args);
}

USER_COMMAND(cmd_lastlog) {
	int opts = 0;

	if (args == NULL)
		return;

	if (*args == '-') {
		if (args[1] == ' ' || args[1] == '\0')
			goto done;

		args++;
		if (*args == '-' && args[1] == ' ') {
			args += 2;
			goto done;
		}

		do {
			switch (*args) {
				case 'b':
					opts |= SWINDOW_FIND_BASIC;
					break;

				case 'i':
					opts |= SWINDOW_FIND_ICASE;
					break;
			}

			if (*++args == ' ') {
				args++;
				break;
			}
		} while (*args != '\0');
	}
done:

	if (*args != '\0')
		imwindow_buffer_find(cur_window(), args, opts);
}

USER_COMMAND(cmd_load) {
	int quiet;
	char buf[PATH_MAX];

	if (args == NULL)
		return;

	quiet = screen_set_quiet(1);

	expand_path(args, buf, sizeof(buf));
	if (read_conf(acct, buf) != 0)
		screen_err_msg("Error reading %s: %s", buf, strerror(errno));

	screen_set_quiet(quiet);
}

USER_COMMAND(cmd_lport) {
	if (args == NULL) {
		screen_cmd_output("New connections will use local port %u",
			ntohs(local_port));
		return;
	}

	if (get_port(args, &local_port) != 0) {
		screen_err_msg("Error: Invalid local port: %s", args);
		return;
	}

	local_port = htons(local_port);
	screen_cmd_output("New connections will use local port %s", args);
}

USER_COMMAND(cmd_me) {
	struct imwindow *win = cur_window();

	if (args == NULL)
		return;

	if (win->type == WIN_TYPE_PRIVMSG)
		pork_action_send(acct, cur_window()->target, args);
	else if (win->type == WIN_TYPE_CHAT) {
		struct chatroom *chat;

		chat = win->data;
		if (chat != NULL)
			chat_send_action(acct, win->data, chat->title, args);
	}
}

USER_COMMAND(cmd_msg) {
	char *target;
	struct chatroom *chat;

	if (args == NULL || !acct->connected)
		return;

	target = strsep(&args, " ");
	if (target == NULL || args == NULL)
		return;

	chat = chat_find(acct, target);
	if (chat != NULL)
		chat_send_msg(acct, chat, target, args);
	else
		pork_msg_send(acct, target, args);
}

USER_COMMAND(cmd_query) {
	struct imwindow *imwindow = cur_window();

	if (args != NULL && !blank_str(args)) {
		struct imwindow *conv_window;

		screen_make_query_window(acct, args, &conv_window);
		screen_goto_window(conv_window->refnum);
	} else
		screen_close_window(imwindow);
}

USER_COMMAND(cmd_quit) {
	if (!event_generate(acct->events, EVENT_QUIT, args))
		pork_exit(0, args, NULL);
}

USER_COMMAND(cmd_refresh) {
	screen_refresh();
}

USER_COMMAND(cmd_save) {
#if 0
	if (save_global_config() == 0)
		screen_cmd_output("Your configuration has been saved");
	else
		screen_err_msg("There was an error saving your configuration");
#endif
}

USER_COMMAND(cmd_send) {
	struct imwindow *imwindow = cur_window();

	if (args == NULL || !acct->connected)
		return;

	if (imwindow->type == WIN_TYPE_PRIVMSG)
		pork_msg_send(acct, imwindow->target, args);
	else if (imwindow->type == WIN_TYPE_CHAT) {
		struct chatroom *chat = imwindow->data;

		if (chat == NULL) {
			screen_err_msg("%s is not a member of %s",
				acct->username, imwindow->target);
		} else
			chat_send_msg(acct, chat, chat->title, args);
	}
}

USER_COMMAND(cmd_unbind) {
	char *binding;
	struct imwindow *imwindow = cur_window();
	struct key_binds *target_binds = imwindow->active_binds;
	int c;

	binding = strsep(&args, " ");
	if (binding == NULL || blank_str(binding))
		return;

	if (binding[0] == '-' && binding[1] != '\0') {
		if (!strcasecmp(binding, "-b") || !strcasecmp(binding, "-buddy"))
			target_binds = &screen.binds.blist;
		else if (!strcasecmp(binding, "-m") || !strcasecmp(binding, "-main"))
			target_binds = &screen.binds.main;
		else {
			screen_err_msg("Bad unbind flag: %s", binding);
			return;
		}

		binding = strsep(&args, " ");

		if (binding == NULL || blank_str(binding))
			return;
	}

	c = bind_get_keycode(binding);
	if (c == -1) {
		screen_err_msg("Bad keycode: %s", binding);
		return;
	}

	if (bind_remove(target_binds, c) == -1)
		screen_cmd_output("There is no binding for %s", binding);
	else
		screen_cmd_output("Binding for %s removed", binding);
}

USER_COMMAND(cmd_unalias) {
	if (args == NULL)
		return;

	if (alias_remove(&screen.alias_hash, args) == -1)
		screen_cmd_output("No such alias: %s", args);
	else
		screen_cmd_output("Alias %s removed", args);
}

USER_COMMAND(cmd_nick) {
	if (args == NULL || blank_str(args))
		return;

	pork_change_nick(acct, args);
}

USER_COMMAND(cmd_notice) {
	char *target;
	struct chatroom *chat;

	if (args == NULL || !acct->connected)
		return;

	target = strsep(&args, " ");
	if (target == NULL || args == NULL)
		return;

	chat = chat_find(acct, target);
	if (chat != NULL)
		chat_send_notice(acct, chat, target, args);
	else
		pork_notice_send(acct, target, args);
}

USER_COMMAND(cmd_perl) {
	size_t num_args = 0;
	char *p = args;
	char **perl_args;
	char *function;
	char *orig;
	size_t i = 0;

	if (args == NULL || blank_str(args))
		return;

	p = args;
	while (*p == ' ')
		p++;

	if (*p == '$')
		p++;
	args = p;
	orig = p;

	function = args;
	p = strchr(function, '(');
	if (p != NULL) {
		*p++ = '\0';
		args = p;
	}

	p = strchr(function, ' ');
	if (p != NULL) {
		*p = '\0';
		args = p + 1;
	}

	if (args == orig) {
		execute_perl(function, NULL);
		return;
	}

	p = args;
	while ((p = strchr(p, ',')) != NULL) {
		++num_args;
		++p;
	}
	num_args += 2;

	p = strchr(args, ')');
	if (p != NULL)
		*p = '\0';

	perl_args = xcalloc(num_args, sizeof(char *));

	while ((p = strsep(&args, ",")) != NULL) {
		char *s;

		while (*p == ' ' || *p == '\t')
			p++;

		s = strrchr(p, ' ');
		if (s != NULL && blank_str(s)) {
			while (*s == ' ')
				s--;
			s[1] = '\0';
		}

		perl_args[i++] = p;
	}

	perl_args[i] = NULL;

	execute_perl(function, perl_args);
	free(perl_args);
}

/*
** This destroys the perl state.
** It has the effect of unloads all scripts. All scripts should
** be catching the EVENT_UNLOAD event and doing any necessary cleanup
** when it happens.
*/

USER_COMMAND(cmd_perl_dump) {
	/*
	** If events are ever made per-account this will have to change.
	*/
	event_generate(acct->events, EVENT_UNLOAD);

	perl_destroy();
	perl_init();
}

USER_COMMAND(cmd_perl_load) {
	int ret;
	char buf[PATH_MAX];

	if (args == NULL)
		return;

	expand_path(args, buf, sizeof(buf));
	ret = perl_load_file(buf);
	if (ret != 0)
		screen_err_msg("Error: The file %s couldn't be loaded", buf);
}

USER_COMMAND(cmd_ping) {
	if (acct->proto->ping != NULL)
		acct->proto->ping(acct, args);
}

USER_COMMAND(cmd_profile) {
	pork_set_profile(acct, args);
}

USER_COMMAND(cmd_event) {
	if (args != NULL)
		run_one_command(acct, args, CMDSET_EVENT);
	else
		run_one_command(acct, "list", CMDSET_EVENT);
}

USER_COMMAND(cmd_acct) {
	if (args != NULL)
		run_one_command(acct, args, CMDSET_ACCT);
	else
		run_one_command(acct, "list", CMDSET_ACCT);
}

USER_COMMAND(cmd_chat) {
	if (!acct->connected)
		return;

	if (args != NULL)
		run_one_command(acct, args, CMDSET_CHAT);
	else
		run_one_command(acct, "list", CMDSET_CHAT);
}

USER_COMMAND(cmd_win) {
	if (args != NULL)
		run_one_command(acct, args, CMDSET_WIN);
	else
		run_one_command(acct, "list", CMDSET_WIN);
}

USER_COMMAND(cmd_file) {
	if (!acct->can_connect)
		return;

	if (args != NULL)
		run_one_command(acct, args, CMDSET_FILE);
	else
		run_one_command(acct, "list", CMDSET_FILE);
}

USER_COMMAND(cmd_buddy) {
	if (!acct->connected)
		return;

	if (args != NULL)
		run_one_command(acct, args, CMDSET_BUDDY);
	else
		run_one_command(acct, "list", CMDSET_BUDDY);
}

USER_COMMAND(cmd_blist) {
	if (!acct->connected || acct->blist == NULL)
		return;

	if (args != NULL)
		run_one_command(acct, args, CMDSET_BLIST);
}

USER_COMMAND(cmd_input) {
	if (args != NULL)
		run_one_command(acct, args, CMDSET_INPUT);
}

USER_COMMAND(cmd_history) {
	if (args != NULL)
		run_one_command(acct, args, CMDSET_HISTORY);
	else
		run_one_command(acct, "list", CMDSET_HISTORY);
}

USER_COMMAND(cmd_scroll) {
	if (args != NULL)
		run_one_command(acct, args, CMDSET_SCROLL);
}

USER_COMMAND(cmd_timer) {
	if (args != NULL)
		run_one_command(acct, args, CMDSET_TIMER);
	else
		run_one_command(acct, "list", CMDSET_TIMER);
}

USER_COMMAND(cmd_set) {
	opt_set_var(screen.global_prefs, args);
}

inline int run_command(struct pork_acct *acct, char *str) {
	return (run_one_command(acct, str, CMDSET_MAIN));
}

int run_mcommand(struct pork_acct *acct, char *str) {
	int i = 0;
	char *copystr = xstrdup(str);
	char *cmdstr = copystr;
	char *curcmd;

	curcmd = strsep(&cmdstr, ";");
	if (curcmd == NULL)
		i = run_one_command(acct, cmdstr, CMDSET_MAIN);
	else {
		while (curcmd != NULL && i != -1) {
			char cmdchars = opt_get_char(screen.global_prefs, OPT_CMDCHARS);

			while (*curcmd == ' ')
				curcmd++;

			while (*curcmd == cmdchars)
				curcmd++;

			i = run_one_command(acct, curcmd, CMDSET_MAIN);
			curcmd = strsep(&cmdstr, ";");
		}
	}

	free(copystr);
	return (i);
}

static int run_one_command(struct pork_acct *acct, char *str, u_int32_t set) {
	char *cmd_str;
	struct command *cmd;

	if (set == CMDSET_MAIN) {
		int ret;
		char *alias_str;

		ret = alias_resolve(&screen.alias_hash, str, &alias_str);
		if (ret == 0)
			str = alias_str;
		else if (ret == 1) {
			screen_err_msg("The alias chain is too long");
			return (-1);
		}
	}

	cmd_str = strsep(&str, " \t");

	cmd = bsearch(cmd_str, command_set[set].set, command_set[set].elem,
			sizeof(struct command), cmd_compare);

	if (cmd == NULL) {
		if (set == CMDSET_MAIN && proto_get_name(cmd_str) != NULL) {
			cmd_str = strsep(&str, " \t");

			cmd = bsearch(cmd_str, acct->proto->cmd, acct->proto->num_cmds,
					sizeof(struct command), cmd_compare);

			if (cmd == NULL) {
				screen_err_msg("Unknown %s command: %s (%s)",
					acct->proto->name, cmd_str, str);
				return (-1);
			}
		} else {
			screen_err_msg("Unknown %scommand: %s",
				command_set[set].type, cmd_str);
			return (-1);
		}
	}

	cmd->cmd(acct, str);
	return (0);
}

static void print_binding(void *data, void *nothing __notused) {
	struct binding *binding = data;
	char key_name[64];

	bind_get_keyname(binding->key, key_name, sizeof(key_name));
	screen_cmd_output("%s is bound to %s", key_name, binding->binding);
}

static void print_alias(void *data, void *nothing __notused) {
	struct alias *alias = data;

	screen_cmd_output("%s is aliased to %s%s",
		alias->alias, alias->orig, (alias->args != NULL ? alias->args : ""));
}

static int cmd_compare(const void *l, const void *r) {
	char *key = (char *) l;
	struct command *cmd = (struct command *) r;

	return (strcasecmp(key, cmd->name));
}

static void print_timer(void *data, void *nothing __notused) {
	struct timer_entry *timer = data;

	screen_cmd_output("[refnum: %u] %d %u %s", timer->refnum,
		(int) timer->interval, timer->times, timer->command);
}

USER_COMMAND(cmd_input_find_next_cmd) {
	u_int32_t cur_pos;
	u_int32_t begin_completion_pos;
	u_int32_t word_begin = 0;
	u_int32_t end_word;
	size_t elements = 0;
	char *input_buf;
	struct pork_input *input;
	struct command *cmd = NULL;

	input = cur_window()->input;
	cur_pos = input->cur - input->prompt_len;
	input_buf = input_get_buf_str(input);

	if (input->begin_completion <= input->prompt_len ||
		input->begin_completion >= input->cur)
	{
		if (cur_pos == 0) {
			/*
			** If you complete from the very beginning of the line,
			** insert a '/' because we only complete commands.
			*/

			input_insert(input, '/');
			input_buf = input_get_buf_str(input);
			cur_pos++;
		} else
			input->begin_completion = input->cur;
	}

	begin_completion_pos = input->begin_completion - input->prompt_len;

	/* Only complete if the line is a command. */
	if (input_buf[0] != '/')
		return;

	end_word = strcspn(&input_buf[1], " \t") + 1;

	if (end_word < begin_completion_pos) {
		size_t i;

		for (i = 1 ; i < array_elem(command_set) ; i++) {
			if (!strncasecmp(command_set[i].type, &input_buf[1], end_word)) {
				elements = command_set[i].elem;
				cmd = command_set[i].set;
				word_begin = end_word;

				break;
			}
		}

		if (word_begin == 0) {
			struct pork_proto *proto = acct->proto;

			if (!strncasecmp(proto->name, &input_buf[1], end_word)) {
				elements = proto->num_cmds;
				cmd = proto->cmd;
				word_begin = end_word;
			} else
				return;
		}

		while (	input_buf[word_begin] == ' ' ||
				input_buf[word_begin] == '\t')
		{
			word_begin++;
		}
	} else {
		word_begin = 1;
		elements = command_set[CMDSET_MAIN].elem;
		cmd = command_set[CMDSET_MAIN].set;
	}

	if (word_begin > 0) {
		int chosen = -1;
		size_t i;

		for (i = 0 ; i < elements ; i++) {
			const struct command *cmd_ptr;

			cmd_ptr = &cmd[i];
			if (!strcmp(cmd_ptr->name, ""))
				continue;

			if (input->cur == input->begin_completion) {
				if (!strncasecmp(cmd_ptr->name,
						&input_buf[word_begin], cur_pos - word_begin))
				{
					/* Don't choose this one if it's already an exact match. */
					if (strlen(cmd_ptr->name) == cur_pos - word_begin)
						continue;

					/* Match */
					chosen = i;
					i = elements;
					continue;
				}
			} else {
				/*
				** We've _already_ matched one. Now get
				** the next in the list, if possible.
				*/

				if (!strncasecmp(cmd_ptr->name,
						&input_buf[word_begin], cur_pos - word_begin))
				{
					/*
					** We've found the one we've matched already. If there
					** will be another possible, it will be the very next
					** one in the list. Let's check that. If it doesn't
					** match, none will.
					*/

					i++;
					if (i < elements) {
						cmd_ptr = &cmd[i];

						if (!strncasecmp(cmd_ptr->name,
								&input_buf[word_begin],
								begin_completion_pos - word_begin))
						{
							chosen = i;
						}

						i = elements;
						continue;
					}
				}
			}
		}

		if (input->begin_completion != input->cur) {
			/*
			** Remove characters added by last match. In
			** other words, remove characters from input->cur
			** until input->begin_completion == input->cur.
			*/
			size_t num_to_remove;

			num_to_remove = input->cur - input->begin_completion;

			for (i = 0 ; i < num_to_remove ; i++)
				input_bkspace(input);
		}

		if (chosen != -1) {
			/* We've found a match. */
			const struct command *cmd_ptr;
			u_int16_t remember_begin = input->begin_completion;

			cmd_ptr = &cmd[chosen];

			input_insert_str(input,
				&cmd_ptr->name[begin_completion_pos - word_begin]);
			input->begin_completion = remember_begin;
		}
	}
}
