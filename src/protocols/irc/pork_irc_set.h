/*
** pork_irc_set.h - /irc set command implementation.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_IRC_SET_H
#define __PORK_IRC_SET_H

struct irc_session;

int irc_init_prefs(struct pork_acct *acct);
struct pref_val *irc_get_default_prefs(void);

enum {
	IRC_OPT_CTCP_BLOCK_ALL = 0,
	IRC_OPT_CTCP_BLOCK_LEAKS,
	IRC_OPT_FORMAT_CHAT_CREATED,
	IRC_OPT_FORMAT_CTCP_REPLY,
	IRC_OPT_FORMAT_CTCP_REQUEST,
	IRC_OPT_FORMAT_USER_MODE,
	IRC_OPT_FORMAT_USERS,
	IRC_OPT_FORMAT_WHOIS_CHANNELS,
	IRC_OPT_FORMAT_WHOIS_IRCNAME,
	IRC_OPT_FORMAT_WHOIS_NICK,
	IRC_OPT_FORMAT_WHOIS_OPERATOR,
	IRC_OPT_FORMAT_WHOIS_SERVER,
	IRC_OPT_IRCHOST,
	IRC_OPT_IRCNAME,
	IRC_OPT_IRCPORT,
	IRC_OPT_QUIT_MSG,
	IRC_OPT_SERVER_LIST,
	IRC_OPT_USERMODE,
	IRC_OPT_USERNAME,
	IRC_NUM_OPTS	
};

#define DEFAULT_IRC_CTCP_BLOCK_ALL			0
#define DEFAULT_IRC_CTCP_BLOCK_LEAKS		0
#define DEFAULT_IRC_FORMAT_CHAT_CREATED		""
#define DEFAULT_IRC_FORMAT_CTCP_REPLY		NULL
#define DEFAULT_IRC_FORMAT_CTCP_REQUEST		NULL
#define DEFAULT_IRC_FORMAT_USER_MODE		NULL
#define DEFAULT_IRC_FORMAT_USERS			NULL
#define DEFAULT_IRC_FORMAT_WHOIS_CHANNELS	NULL
#define DEFAULT_IRC_FORMAT_WHOIS_IRCNAME	NULL
#define DEFAULT_IRC_FORMAT_WHOIS_NICK		NULL
#define DEFAULT_IRC_FORMAT_WHOIS_OPERATOR	NULL
#define DEFAULT_IRC_FORMAT_WHOIS_SERVER		NULL
#define	DEFAULT_IRC_IRCHOST					NULL
#define DEFAULT_IRC_IRCNAME					"*Unknown*"
#define DEFAULT_IRC_IRCPORT					6667
#define DEFAULT_IRC_QUIT_MSG				"eat the piggy [pork: http://dev.ojnk.net]"
#define DEFAULT_IRC_SERVER_LIST				NULL
#define DEFAULT_IRC_USERMODE				"+i"
#define DEFAULT_IRC_USERNAME				NULL

#endif
