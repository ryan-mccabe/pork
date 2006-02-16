/*
** pork_perl_xs.c - Perl scripting support
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include <stdlib.h>
#include <sys/mman.h>

#undef PACKAGE
#undef instr

#ifndef HAS_BOOL
#	define HAS_BOOL
#endif

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_set.h>
#include <pork_imsg.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_acct.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_alias.h>
#include <pork_conf.h>
#include <pork_msg.h>
#include <pork_command.h>
#include <pork_perl.h>
#include <pork_perl_xs.h>

extern struct screen screen;

XS(PORK_run_cmd) {
	char *command_str;
	size_t notused;
	u_int32_t i = 0;
	dXSARGS;

	(void) cv;

	if (items < 1)
		XSRETURN_IV(-1);

	while (items-- > 0) {
		command_str = SvPV(ST(i), notused);
		i++;

		if (command_str == NULL)
			continue;

		run_command(cur_window()->owner, command_str);
	}

	XSRETURN_IV(0);
}

XS(PORK_connect) {
	char *username = NULL;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 0)
		username = SvPV(ST(0), notused);

	/*
	FIXME
	cmd_connect(cur_window()->owner, username);
	XSRETURN_IV(0);
	*/

	XSRETURN_IV(-1);
}

XS(PORK_disconnect) {
	struct pork_acct *acct;
	char *reason = NULL;
	dXSARGS;

	(void) cv;

	if (items >= 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);

		if (items >= 2) {
			size_t notused;

			reason = SvPV(ST(1), notused);
		}
	} else
		acct = cur_window()->owner;

	if (!acct->can_connect)
		XSRETURN_IV(-1);

	XSRETURN_IV(pork_acct_del_refnum(acct->refnum, reason));
}

XS(PORK_cur_user) {
	dXSARGS;

	(void) cv;
	(void) items;
	XSRETURN_PV(cur_window()->owner->username);
}

XS(PORK_err_msg) {
	size_t notused;
	char *msg;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_IV(-1);

	msg = SvPV(ST(0), notused);
	if (msg == NULL)
		XSRETURN_IV(-1);

	screen_err_msg("%s", msg);
	XSRETURN_IV(0);
}

XS(PORK_prompt_user) {
	dXSARGS;
	char buf[4096];
	int ret;
	char *prompt = NULL;

	(void) cv;

	if (items > 0) {
		size_t notused;

		prompt = SvPV(ST(0), notused);
	}

	screen_prompt_user(prompt, buf, sizeof(buf));
	ret = strlen(buf);

	if (ret <= 0)
		XSRETURN_UNDEF;

	XSRETURN_PV(buf);
}

XS(PORK_status_msg) {
	size_t notused;
	char *msg;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_IV(-1);

	msg = SvPV(ST(0), notused);
	if (msg == NULL)
		XSRETURN_IV(-1);

	screen_win_msg(cur_window(), 1, 1, 1, MSG_TYPE_STATUS, "%s", msg);
	XSRETURN_IV(0);
}

XS(PORK_load) {
	int args = 0;
	dXSARGS;

	(void) cv;

	if (items < 1)
		XSRETURN_IV(-1);

	while (items > 0) {
		char *file;
		size_t notused;

		file = SvPV(ST(args), notused);
		if (file == NULL)
			XSRETURN_IV(-1);

		if (read_conf(cur_window()->owner, file) != 0)
			XSRETURN_IV(-1);

		args++;
		items--;
	}

	XSRETURN_IV(0);
}

XS(PORK_load_perl) {
	int args = 0;
	dXSARGS;

	(void) cv;

	if (items < 1)
		XSRETURN_IV(-1);

	while (items > 0) {
		char *file;
		size_t notused;

		file = SvPV(ST(args), notused);
		if (file == NULL)
			XSRETURN_IV(-1);

		if (perl_load_file(file) != 0)
			XSRETURN_IV(-1);

		args++;
		items--;
	}

	XSRETURN_IV(0);
}

XS(PORK_echo) {
	size_t notused;
	char *msg;
	dXSARGS;

	(void) cv;

	if (items < 1)
		XSRETURN_IV(-1);

	msg = SvPV(ST(0), notused);
	screen_win_msg(cur_window(), 0, 0, 1, MSG_TYPE_CMD_OUTPUT, msg);
	XSRETURN_IV(0);
}

XS(PORK_quit) {
	int exit_val = 0;
	char *reason = NULL;
	dXSARGS;

	(void) cv;

	if (items >= 1)
		exit_val = SvIV(ST(0));

	if (items >= 2) {
		size_t notused;

		reason = SvPV(ST(1), notused);
	}

	pork_exit(exit_val, reason, NULL);
	XSRETURN_IV(0);
}

XS(PORK_refresh) {
	dXSARGS;

	(void) cv;
	(void) items;

	screen_refresh();
	XSRETURN_IV(0);
}

XS(PORK_save) {
	dXSARGS;

	(void) cv;
	(void) items;

	XSRETURN_IV(-1);
//	XSRETURN_IV(save_global_config());
}

XS(PORK_bind) {
	char *key_str;
	int key;
	char *command;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 2)
		XSRETURN_IV(-1);

	key_str = SvPV(ST(0), notused);
	command = SvPV(ST(1), notused);

	if (key_str == NULL || command == NULL)
		XSRETURN_IV(-1);

	key = bind_get_keycode(key_str);
	if (key == -1)
		XSRETURN_IV(-1);

	bind_add(&screen.binds.main, key, command);
	XSRETURN_IV(0);
}

XS(PORK_unbind) {
	char *key_str;
	int key;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_IV(-1);

	key_str = SvPV(ST(0), notused);
	if (key_str == NULL)
		XSRETURN_IV(-1);

	key = bind_get_keycode(key_str);
	if (key == -1)
		XSRETURN_IV(-1);

	key = bind_remove(&screen.binds.main, key);
	XSRETURN_IV(0);
}

XS(PORK_bind_get) {
	char *key_str;
	int key;
	size_t notused;
	struct binding *binding;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_EMPTY;

	key_str = SvPV(ST(0), notused);
	if (key_str == NULL)
		XSRETURN_EMPTY;

	key = bind_get_keycode(key_str);
	if (key == -1)
		XSRETURN_EMPTY;

	binding = bind_find(&screen.binds.main, key);
	if (binding == NULL)
		XSRETURN_EMPTY;

	XSRETURN_PV(binding->binding);
}

XS(PORK_blist_bind) {
	char *key_str;
	int key;
	char *command;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 2)
		XSRETURN_IV(-1);

	key_str = SvPV(ST(0), notused);
	command = SvPV(ST(1), notused);

	if (key_str == NULL || command == NULL)
		XSRETURN_IV(-1);

	key = bind_get_keycode(key_str);
	if (key == -1)
		XSRETURN_IV(-1);

	bind_add(&screen.binds.blist, key, command);
	XSRETURN_IV(0);
}

XS(PORK_blist_unbind) {
	char *key_str;
	int key;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_IV(-1);

	key_str = SvPV(ST(0), notused);
	if (key_str == NULL)
		XSRETURN_IV(-1);

	key = bind_get_keycode(key_str);
	if (key == -1)
		XSRETURN_IV(-1);

	key = bind_remove(&screen.binds.blist, key);
	XSRETURN_IV(0);
}

XS(PORK_blist_bind_get) {
	char *key_str;
	int key;
	size_t notused;
	struct binding *binding;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_EMPTY;

	key_str = SvPV(ST(0), notused);
	if (key_str == NULL)
		XSRETURN_EMPTY;

	key = bind_get_keycode(key_str);
	if (key == -1)
		XSRETURN_EMPTY;

	binding = bind_find(&screen.binds.blist, key);
	if (binding == NULL)
		XSRETURN_EMPTY;

	XSRETURN_PV(binding->binding);
}

XS(PORK_alias) {
	char *alias;
	char *cmd;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 2)
		XSRETURN_IV(-1);

	alias = SvPV(ST(0), notused);
	cmd = SvPV(ST(1), notused);

	if (alias == NULL || cmd == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(alias_add(&screen.alias_hash, alias, cmd));
}

XS(PORK_alias_get) {
	char *str;
	size_t notused;
	struct alias *alias;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_EMPTY;

	str = SvPV(ST(0), notused);
	if (str == NULL)
		XSRETURN_EMPTY;

	alias = alias_find(&screen.alias_hash, str);
	if (alias == NULL)
		XSRETURN_EMPTY;

	SP -= items;
	XPUSHs(sv_2mortal(newSVpv(alias->orig, 0)));

	if (alias->args != NULL) {
		XPUSHs(sv_2mortal(newSVpv(alias->args, 0)));
		XSRETURN(2);
	}

	XSRETURN(1);
}

XS(PORK_unalias) {
	char *alias;
	size_t notused;
	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_IV(-1);

	alias = SvPV(ST(0), notused);
	if (alias == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(alias_remove(&screen.alias_hash, alias));
}

XS(PORK_get_cur_user) {
	dXSARGS;

	(void) cv;
	(void) items;

	XSRETURN_PV(cur_window()->owner->username);
}

#if 0
XS(PORK_set_opt) {
	char *var;
	char *val;
	size_t notused;
	int opt;
	dXSARGS;

	(void) cv;

	if (items != 2)
		XSRETURN_IV(-1);

	var = SvPV(ST(0), notused);
	val = SvPV(ST(1), notused);

	if (var == NULL || var == NULL)
		XSRETURN_IV(-1);

	opt = opt_find(var);
	if (opt == -1)
		XSRETURN_IV(-1);

	XSRETURN_IV(opt_set(opt, val));
}

XS(PORK_get_opt) {
	char *val;
	size_t notused;
	char buf[2048];

	dXSARGS;

	(void) cv;

	if (items != 1)
		XSRETURN_EMPTY;

	val = SvPV(ST(0), notused);
	if (val == NULL)
		XSRETURN_EMPTY;

	if (opt_get_val(val, buf, sizeof(buf)) == -1)
		XSRETURN_EMPTY;

	XSRETURN_PV(buf);
}

#endif

/* XXX - fix */

XS(PORK_get_acct_list) {
	dlist_t *cur = screen.acct_list;
	int args = 0;
	dXSARGS;

	(void) cv;

	if (items != 0 || cur == NULL)
		XSRETURN_EMPTY;

	SP -= items;
	do {
		struct pork_acct *acct = cur->data;

		if (acct->can_connect) {
			XPUSHs(sv_2mortal(newSVpv(acct->username, 0)));
			args++;
		}
		cur = cur->next;
	} while (cur != NULL);

	XSRETURN(args);
}

XS(PORK_set_profile) {
	size_t notused;
	char *profile;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	profile = SvPV(ST(0), notused);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	free(acct->profile);
	if (profile != NULL)
		acct->profile = xstrdup(profile);
	else
		acct->profile = NULL;

	if (!acct->connected)
		XSRETURN_IV(-1);

	XSRETURN_IV(pork_set_profile(acct, profile));
}

XS(PORK_get_profile) {
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 0)
		XSRETURN_EMPTY;

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_EMPTY;
	} else
		acct = cur_window()->owner;

	if (acct->profile == NULL)
		XSRETURN_EMPTY;

	XSRETURN_PV(acct->profile);
}

XS(PORK_send_msg) {
	size_t notused;
	char *dest;
	char *msg;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	dest = SvPV(ST(0), notused);
	msg = SvPV(ST(1), notused);

	if (dest == NULL || msg == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	if (!acct->connected)
		XSRETURN_IV(-1);

	return (XSRETURN_IV(pork_msg_send(acct, dest, msg)));
}

XS(PORK_send_msg_auto) {
	size_t notused;
	char *dest;
	char *msg;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	dest = SvPV(ST(0), notused);
	msg = SvPV(ST(1), notused);

	if (dest == NULL || msg == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	if (!acct->connected)
		XSRETURN_IV(-1);

	return (XSRETURN_IV(pork_msg_autoreply(acct, dest, msg)));
}

XS(PORK_set_away) {
	size_t notused;
	char *msg = NULL;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items > 2)
		XSRETURN_IV(-1);

	if (items >= 1 && SvTRUE(ST(0)))
		msg = SvPV(ST(0), notused);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	if (!acct->connected)
		XSRETURN_IV(-1);

	XSRETURN_IV(pork_set_away(acct, msg));
}

XS(PORK_set_idle) {
	int seconds;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	seconds = SvIV(ST(0));

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	if (!acct->connected)
		XSRETURN_IV(-1);

	XSRETURN_IV(pork_set_idle_time(acct, seconds));
}

XS(PORK_warn) {
	size_t notused;
	char *target;
	int anon;
	struct pork_acct *acct;
	int ret;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	anon = SvIV(ST(1));

	if (target == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	if (!acct->connected)
		XSRETURN_IV(-1);

	if (anon != 0)
		ret = pork_send_warn(acct, target);
	else
		ret = pork_send_warn_anon(acct, target);

	XSRETURN_IV(ret);
}
