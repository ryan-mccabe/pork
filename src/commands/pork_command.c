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
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_imwindow_set.h>
#include <pork_input_set.h>
#include <pork_slist.h>
#include <pork_buddy_list.h>
#include <pork_proto.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_acct_set.h>
#include <pork_cstr.h>
#include <pork_misc.h>
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
#include <pork_help.h>

extern struct sockaddr_storage local_addr;
extern in_port_t local_port;

static int cmd_compare(const void *l, const void *r) {
	char *key = (char *) l;
	struct command *cmd = (struct command *) r;

	return (strcasecmp(key, cmd->name));
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
		struct pork_proto *proto;

		if (set == CMDSET_MAIN && (proto = proto_get_name(cmd_str)) != NULL) {
			cmd_str = strsep(&str, " \t");

			if (cmd_str != NULL && cmd_str[0] != '\0') {
				cmd = bsearch(cmd_str, proto->cmd, proto->num_cmds,
						sizeof(struct command), cmd_compare);

				if (cmd == NULL) {
					screen_err_msg("Unknown %s command: %s",
						proto->name, cmd_str);
					return (-1);
				}

				if (proto != acct->proto) {
					/* yeah, this isn't a hack at all. */
					if (!strcasecmp(cmd->name, "set") ||
						!strcasecmp(cmd->name, "save"))
					{
						cmd->cmd(NULL, str);
						return (0);
					} else {
						screen_err_msg("%s may not run %s commands",
							acct->username, proto->name);
						return (-1);
					}
				}
			} else {
				if (str != NULL)
					screen_err_msg("Unknown %s command: %s", proto->name, str);

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

static int command_send_to_win(struct pork_acct *acct, char *str) {
	struct imwindow *imwindow = cur_window();

	if (str == NULL || !acct->connected)
		return (-1);

	if (imwindow->type == WIN_TYPE_PRIVMSG)
		pork_msg_send(acct, imwindow->target, str);
	else if (imwindow->type == WIN_TYPE_CHAT) {
		struct chatroom *chat = imwindow->data;

		if (chat == NULL) {
			screen_err_msg("%s is not a member of %s",
				acct->username, imwindow->target);
		} else
			chat_send_msg(acct, chat, chat->title, str);
	}
}

int command_enter_str(struct pork_acct *acct, char *str) {
	if (!event_generate(acct->events, EVENT_SEND_LINE,
		str, acct->refnum))
	{
		return (-1);
	}

	if (str[0] == opt_get_char(screen.global_prefs, OPT_CMDCHARS))
		run_command(acct, &str[1]);
	else
		command_send_to_win(acct, str);
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
	FIXME
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
	dlist_t *cur;

	for (cur = screen.acct_list ; cur != NULL ; cur = cur->next) {
		struct pork_acct *a = cur->data;
		event_generate(acct->events, EVENT_UNLOAD, a->refnum);
	}

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


/*
** The / command set.
**
** Note that the "struct command" arrays are arranged in alphabetical
** order. They have to be like that.
*/

USER_COMMAND(cmd_complete);
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
	{ "complete",	cmd_complete		},
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

static struct command_set {
	struct command *set;
	size_t elem;
	char *type;
} command_set[] = {
	{	command,			array_elem(command),			""			},
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
** This needs to follow the command_set declaration.
*/

USER_COMMAND(cmd_complete) {
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
