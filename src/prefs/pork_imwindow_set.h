/*
** pork_imwindow_set.h - /win SET command implementation.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_IMWINDOW_SET_H
#define __PORK_IMWINDOW_SET_H

struct imwindow;

int imwindow_init_prefs(struct imwindow *win);
struct pref_val *imwindow_get_default_prefs(void);

enum {
	WIN_OPT_ACTIVITY_TYPES = 0,
	WIN_OPT_AUTOSAVE,
	WIN_OPT_BEEP,
	WIN_OPT_BEEP_MAX,
	WIN_OPT_BEEP_ON_OUTPUT,
	WIN_OPT_LOG,
	WIN_OPT_LOG_TYPES,
	WIN_OPT_LOGFILE,
	WIN_OPT_PRIVATE_INPUT,
	WIN_OPT_SCROLL_ON_INPUT,
	WIN_OPT_SCROLL_ON_OUTPUT,
	WIN_OPT_SCROLLBUF_LEN,
	WIN_OPT_SHOW_BLIST,
	WIN_OPT_SHOW_BUDDY_AWAY,
	WIN_OPT_SHOW_BUDDY_IDLE,
	WIN_OPT_SHOW_BUDDY_SIGNON,
	WIN_OPT_WORDWRAP,
	WIN_OPT_WORDWRAP_CHAR,
	WIN_NUM_OPTS
};

#define DEFAULT_WIN_ACTIVITY_TYPES				0xffffffff
#define DEFAULT_WIN_AUTOSAVE					0
#define DEFAULT_WIN_BEEP						0
#define DEFAULT_WIN_BEEP_MAX					3
#define DEFAULT_WIN_BEEP_ON_OUTPUT				0
#define DEFAULT_WIN_LOG							0
#define DEFAULT_WIN_LOG_TYPES					0xffffffff
#define DEFAULT_WIN_LOGFILE						NULL
#define DEFAULT_WIN_PRIVATE_INPUT				0
#define DEFAULT_WIN_SCROLL_ON_INPUT				1
#define DEFAULT_WIN_SCROLL_ON_OUTPUT			0
#define DEFAULT_WIN_SCROLLBUF_LEN				5000
#define DEFAULT_WIN_SHOW_BLIST					0
#define DEFAULT_WIN_SHOW_BUDDY_AWAY				1
#define DEFAULT_WIN_SHOW_BUDDY_IDLE				0
#define DEFAULT_WIN_SHOW_BUDDY_SIGNON			1
#define DEFAULT_WIN_WORDWRAP					1
#define DEFAULT_WIN_WORDWRAP_CHAR				' '

#else
#	warning "included multiple times"
#endif
