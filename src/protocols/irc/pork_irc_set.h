/*
** pork_irc_set.h - /irc set command implementation.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
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
	IRC_OPT_AUTOSAVE = 0,
	IRC_OPT_CTCP_BLOCK_ALL,
	IRC_OPT_CTCP_BLOCK_LEAKS,
	IRC_OPT_FORMAT_CHAT_CREATED,
	IRC_OPT_FORMAT_CHAT_TOPIC,
	IRC_OPT_FORMAT_CHAT_TOPIC_INFO,
	IRC_OPT_FORMAT_CTCP_REPLY,
	IRC_OPT_FORMAT_CTCP_REPLY_PING,
	IRC_OPT_FORMAT_CTCP_REQUEST,
	IRC_OPT_FORMAT_KILLED,
	IRC_OPT_FORMAT_USER_MODE,
	IRC_OPT_FORMAT_USERS,
	IRC_OPT_FORMAT_WHOIS_AWAY,
	IRC_OPT_FORMAT_WHOIS_CHANNELS,
	IRC_OPT_FORMAT_WHOIS_IDLE,
	IRC_OPT_FORMAT_WHOIS_IRCNAME,
	IRC_OPT_FORMAT_WHOIS_NICK,
	IRC_OPT_FORMAT_WHOIS_OPERATOR,
	IRC_OPT_FORMAT_WHOIS_SERVER,
	IRC_OPT_FORMAT_WHOIS_SIGNON,
	IRC_OPT_IRCHOST,
	IRC_OPT_IRCNAME,
	IRC_OPT_IRCPORT,
	IRC_OPT_QUIT_MSG,
	IRC_OPT_SERVER_LIST,
	IRC_OPT_USERMODE,
	IRC_OPT_USERNAME,
	IRC_NUM_OPTS
};

#define IRC_OPT_FORMAT_OFFSET IRC_OPT_FORMAT_CHAT_CREATED

#define DEFAULT_IRC_AUTOSAVE				0
#define DEFAULT_IRC_CTCP_BLOCK_ALL			0
#define DEFAULT_IRC_CTCP_BLOCK_LEAKS		0
#define DEFAULT_IRC_FORMAT_CHAT_CREATED		"%c$C%x created%W:%x $T"
#define DEFAULT_IRC_FORMAT_CHAT_MODE		"Mode for %c$C%W:%x $M"
#define DEFAULT_IRC_FORMAT_CHAT_TOPIC		"Topic for %c$C%W:%x $M"
#define DEFAULT_IRC_FORMAT_CHAT_TOPIC_INFO	"Topic for %c$C%x set by %c$N%x at $T"
#define DEFAULT_IRC_FORMAT_CTCP_REPLY		"%YCTCP REPLY %G%s %D[%x$K $M%D]%x from %C$D%D(%c$H%%D)"
#define DEFAULT_IRC_FORMAT_CTCP_REPLY_PING	""
#define DEFAULT_IRC_FORMAT_CTCP_REQUEST		"%WCTCP%M $K %D[%x$M%D]%x from %C$D%D(%c$H%D)%x to %W$N"
#define DEFAULT_IRC_FORMAT_KILLED			"You have been killed by %c$S %D(%c$U%D)%x ($R)"
#define DEFAULT_IRC_FORMAT_USER_MODE		"Mode %c$N%W:%x $M"
#define DEFAULT_IRC_FORMAT_USERS			"Users on %c$C%W:%x $L"
#define DEFAULT_IRC_FORMAT_WHOIS_AWAY		"%D-%m-%M--%Ca%cway%W:%x $M"
#define DEFAULT_IRC_FORMAT_WHOIS_CHANNELS	"%Cc%channels%W:%x $C"
#define DEFAULT_IRC_FORMAT_WHOIS_IRCNAME	"%D-%Ci%crcname%W:%x $N"
#define DEFAULT_IRC_FORMAT_WHOIS_IDLE		"%D-%m-%M--%Ci%cdle%W:%x $T"
#define DEFAULT_IRC_FORMAT_WHOIS_NICK		"%D-%m-%M-%W$N%M-%D(%c$I%W@%c$H%D)%M-%m-%D-"
#define DEFAULT_IRC_FORMAT_WHOIS_OPERATOR	"%Co%cperator%W:%x $N $M"
#define DEFAULT_IRC_FORMAT_WHOIS_SERVER		"%D-%m-%Cs%cerver%W:%x $S %D(%x$X%D)"
#define DEFAULT_IRC_FORMAT_WHOIS_SIGNON		"%D-%m-%Cs%cignon%W:%x $T"
#define	DEFAULT_IRC_IRCHOST					NULL
#define DEFAULT_IRC_IRCNAME					"pork: http://dev.ojnk.net"
#define DEFAULT_IRC_IRCPORT					6667
#define DEFAULT_IRC_QUIT_MSG				"pork: http://dev.ojnk.net"
#define DEFAULT_IRC_SERVER_LIST				NULL
#define DEFAULT_IRC_USERMODE				"+i"
#define DEFAULT_IRC_USERNAME				NULL

#else
#	warning "included multiple times"
#endif
