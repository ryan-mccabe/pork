/*
** pork_irc_set.c - /irc set support
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>
#include <pwd.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_queue.h>
#include <pork_color.h>
#include <pork_acct.h>
#include <pork_set.h>
#include <pork_proto.h>
#include <pork_imwindow.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

#include <pork_irc.h>
#include <pork_irc_set.h>

static const struct pork_pref irc_pref_list[] = {
	{	.name = "CTCP_BLOCK_ALL",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "CTCP_BLOCK_LEAKS",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "FORMAT_CHAT_CREATED",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CTCP_REPLY",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_CTCP_REQUEST",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_USER_MODE",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "FORMAT_USERS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "WHOIS_CHANNELS",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "WHOIS_IRCNAME",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "WHOIS_NICK",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "WHOIS_OPERATOR",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "WHOIS_SERVER",
		.type = OPT_TYPE_FORMAT,
		.set = opt_set_format,
	},{	.name = "IRCHOST",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "IRCNAME",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "IRCPORT",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "PORK_DIR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "QUIT_MSG",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "SERVER_LIST",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "USERMODE",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	},{	.name = "USERNAME",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
	}
};

static const struct pref_set irc_pref_set = {
	.name = "irc",
	.num_opts = IRC_NUM_OPTS,
	.prefs = irc_pref_list
};

static pref_val_t irc_default_pref_vals[] = {
	{	.pref_val.b = DEFAULT_IRC_CTCP_BLOCK_ALL,
	},{	.pref_val.b = DEFAULT_IRC_CTCP_BLOCK_LEAKS,
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_CHAT_CREATED,
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_CTCP_REPLY,
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_CTCP_REQUEST,
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_USER_MODE,
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_USERS,
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_WHOIS_CHANNELS
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_WHOIS_IRCNAME
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_WHOIS_NICK
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_WHOIS_OPERATOR
	},{	.pref_val.s = DEFAULT_IRC_FORMAT_WHOIS_SERVER
	},{	.pref_val.s = DEFAULT_IRC_IRCHOST,
	},{	.pref_val.s = DEFAULT_IRC_IRCNAME,
	},{	.pref_val.i = DEFAULT_IRC_IRCPORT,
	},{	.pref_val.s = DEFAULT_IRC_PORK_DIR,
	},{	.pref_val.s = DEFAULT_IRC_QUIT_MSG,
	},{	.pref_val.s = DEFAULT_IRC_SERVER_LIST,
	},{	.pref_val.s = DEFAULT_IRC_USERMODE,
	},{	.pref_val.s = DEFAULT_IRC_USERNAME,
	}
};

static struct pref_val irc_defaults = {
	.set = &irc_pref_set,
	.val = irc_default_pref_vals
};

int irc_init_prefs(struct pork_acct *acct) {
	struct pref_val *prefs;

	prefs = xmalloc(sizeof(*prefs));
	prefs->set = &irc_pref_set;
	opt_copy_pref_val(prefs, irc_default_pref_vals,
		sizeof(irc_default_pref_vals));

	acct->proto_prefs = prefs;
	return (0);
}

inline struct pref_val *irc_get_default_prefs(void) {
	return (&irc_defaults);
}
