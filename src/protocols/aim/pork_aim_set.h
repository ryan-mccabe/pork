/*
** pork_aim_set.h - /aim set support.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
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
	AIM_NUM_OPTS
};

#define DEFAULT_AIM_AUTOSAVE				0

#else
#	warning "included multiple times"
#endif
