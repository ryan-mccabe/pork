/*
** pork_command.h - interface to commands typed by the user
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_COMMAND_H
#define __PORK_COMMAND_H

struct pork_acct;

#define USER_COMMAND(x)	void x (struct pork_acct *acct __notused, char *args __notused)

enum {
	CMDSET_MAIN,
	CMDSET_WIN,
	CMDSET_HISTORY,
	CMDSET_INPUT,
	CMDSET_SCROLL,
	CMDSET_BUDDY,
	CMDSET_BLIST,
	CMDSET_TIMER,
	CMDSET_EVENT,
	CMDSET_CHAT,
	CMDSET_FILE,
	CMDSET_ACCT,
	CMDSET_PROTO,
};

struct command {
	char *name;
	void (*cmd)(struct pork_acct *, char *);
};

int run_mcommand(struct pork_acct *acct, char *str);
inline int run_command(struct pork_acct *acct, char *str);

#endif
