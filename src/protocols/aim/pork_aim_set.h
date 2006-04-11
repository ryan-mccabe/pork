/*
** pork_aim_set.h - /aim set support.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_AIM_SET_H
#define __PORK_AIM_SET_H

int aim_init_prefs(struct pork_acct *acct);
struct pref_val *aim_get_default_prefs(void);

enum {
	AIM_OPT_AUTOSAVE = 0,
	AIM_OPT_AIM_HOST,
	AIM_OPT_AIM_PORT,
	AIM_OPT_AIM_SERVER,
	AIM_OPT_FORMAT_WHOIS_AWAY,
	AIM_OPT_FORMAT_WHOIS_IDLE,
	AIM_OPT_FORMAT_WHOIS_MEMBER,
	AIM_OPT_FORMAT_WHOIS_NAME,
	AIM_OPT_FORMAT_WHOIS_SIGNON,
	AIM_OPT_FORMAT_WHOIS_USERINFO,
	AIM_OPT_FORMAT_WHOIS_WARNLEVEL,
	AIM_OPT_OUTGOING_HTML_HEADER,
	AIM_OPT_OUTGOING_MSG_FONT,
	AIM_OPT_OUTGOING_MSG_FONT_BGCOLOR,
	AIM_OPT_OUTGOING_MSG_FONT_FGCOLOR,
	AIM_OPT_OUTGOING_MSG_FONT_SIZE,
	AIM_OPT_PORT,
	AIM_OPT_SERVER,
	AIM_NUM_OPTS
};

#define DEFAULT_AIM_AUTOSAVE					0
#define DEFAULT_AIM_AIM_HOST					""
#define DEFAULT_AIM_AIM_PORT					5190
#define DEFAULT_AIM_AIM_SERVER					"login.oscar.aol.com"
#define DEFAULT_AIM_FORMAT_WHOIS_AWAY			"%D-%Ca%cway message%W:%x $A"
#define DEFAULT_AIM_FORMAT_WHOIS_IDLE			"%D-%p-%P--%Ci%cdle time%W:%x $i"
#define DEFAULT_AIM_FORMAT_WHOIS_MEMBER			"%D-%Cm%cember since%W:%x $d"
#define DEFAULT_AIM_FORMAT_WHOIS_NAME			"%D-%p-%P---%Cu%csername%W:%x $N"
#define DEFAULT_AIM_FORMAT_WHOIS_SIGNON			"%D-%Co%cnline since%W:%x $s"
#define DEFAULT_AIM_FORMAT_WHOIS_USERINFO		"%D-%p-%P--%Cu%cser info%W:%x $P"
#define DEFAULT_AIM_FORMAT_WHOIS_WARNLEVEL		"%D-%p-%P-%Cw%carn level%W:%x $W%%"
#define DEFAULT_AIM_OUTGOING_HTML_HEADER		1
#define DEFAULT_AIM_OUTGOING_MSG_FONT			""
#define DEFAULT_AIM_OUTGOING_MSG_FONT_BGCOLOR	"#ffffff"
#define DEFAULT_AIM_OUTGOING_MSG_FONT_FGCOLOR	"#000000"
#define DEFAULT_AIM_OUTGOING_MSG_FONT_SIZE		""

#else
#	warning "included multiple times"
#endif
