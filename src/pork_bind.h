/*
** pork_bind.h - interface to key bindings
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_BIND_H
#define __PORK_BIND_H

struct pork_acct;

enum {
	BINDS_MAIN,
	BINDS_BUDDY
};

struct binding {
	int key;
	char *binding;
};

struct key_binds {
	hash_t hash;
	void (*success)(struct pork_acct *acct, struct binding *binding);
	void (*failure)(int key);
};

struct binds {
	struct key_binds main;
	struct key_binds blist;
};

int bind_init(struct binds *binds);
void bind_destroy(struct binds *binds);
int bind_remove(struct key_binds *bind_set, int c);
int bind_exec(struct pork_acct *acct, struct key_binds *bind_set, int c);
struct binding *bind_find(struct key_binds *bind_set, int c);
void bind_add(struct key_binds *bind_set, int c, char *command);
void bind_set_handlers(	struct key_binds *bind_set,
						void (*success)(struct pork_acct *acct,
										struct binding *binding),
						void (*failure)(int key));

int bind_get_keycode(char *keystr);
void bind_get_keyname(int key, char *result, size_t len);

#else
#	warning "included multiple times"
#endif
