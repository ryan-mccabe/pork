/*
** pork_proto.c
** Copyright (C) 2003-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_set.h>

static struct pork_proto *proto_table[PROTO_MAX + 1];

extern int irc_proto_init(struct pork_proto *proto);
extern int aim_proto_init(struct pork_proto *proto);

inline struct pork_proto *proto_get(int protocol) {
	if (protocol > PROTO_MAX || protocol < -1)
		return (NULL);

	return (proto_table[protocol + 1]);
}

struct pork_proto *proto_get_name(const char *name) {
	size_t i;

	for (i = 1 ; i <= PROTO_MAX ; i++) {
		if (!strcasecmp(name, proto_table[i]->name))
			return (proto_table[i]);
	}

	return (NULL);
}

int proto_new(	int protocol,
				const char *name,
				int (*init_func)(struct pork_proto *))
{
	struct pork_proto *new_proto;
	int ret;

	if (protocol < -1 || protocol > PROTO_MAX ||
		proto_table[protocol + 1] != NULL || init_func == NULL)
	{
		return (-1);
	}

	new_proto = xcalloc(1, sizeof(*new_proto));
	new_proto->protocol = protocol;
	xstrncpy(new_proto->name, name, sizeof(new_proto->name));

	ret = init_func(new_proto);
	if (ret != 0) {
		free(new_proto);
		return (ret);
	}

	proto_table[protocol + 1] = new_proto;
	return (0);
}

int proto_get_num(const char *name) {
	size_t i;

	for (i = 0 ; i <= PROTO_MAX && proto_table[i] != NULL ; i++) {
		if (!strcasecmp(proto_table[i]->name, name))
			return (proto_table[i]->protocol);
	}

	return (-1);
}

void proto_destroy(void) {
	size_t i;

	for (i = 0 ; i <= PROTO_MAX ; i++)
		free(proto_table[i]);
}

int proto_set(struct pork_proto *proto, struct pref_val *pref, char *args) {
	if (proto == NULL || proto->set == NULL || proto->get_default_prefs == NULL)
		return (-1);

	if (pref == NULL)
		pref = proto->get_default_prefs();

	if (args != NULL && !strncasecmp(args, "-default", 8)) {
		args += 8;
		while (args[0] == ' ')
			args++;

		pref = proto->get_default_prefs();
	}

	return (opt_set_var(pref, args, proto));
}

static int proto_init_null(struct pork_proto *proto) {
	proto->normalize = xstrncpy;
	return (0);
}

int proto_init(void) {
	proto_new(PROTO_NULL, "NULL", proto_init_null);
#ifdef AIM_SUPPORT
	proto_new(PROTO_AIM, "AIM", aim_proto_init);
#endif
#ifdef IRC_SUPPORT
	proto_new(PROTO_IRC, "IRC", irc_proto_init);
#endif

	return (0);
}
