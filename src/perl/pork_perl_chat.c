/*
** pork_perl_chat.c - Perl scripting support
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
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
#include <pork_imwindow.h>
#include <pork_acct.h>
#include <pork_screen.h>
#include <pork_chat.h>
#include <pork_perl_xs.h>

XS(PORK_chat_ban) {
	char *chat_name;
	char *ban_user;
	size_t notused;
	struct pork_acct *acct;
	struct chatroom *chat;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	chat_name = SvPV(ST(0), notused);
	ban_user = SvPV(ST(1), notused);

	if (chat_name == NULL || ban_user == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	chat = chat_find(acct, chat_name);
	if (chat == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(chat_ban(acct, chat, ban_user));
}

XS(PORK_chat_kick) {
	char *chat_name;
	char *user;
	char *reason = NULL;
	size_t notused;
	struct pork_acct *acct;
	struct chatroom *chat;
	dXSARGS;

	(void) cv;

	if (items < 2)
		XSRETURN_IV(-1);

	chat_name = SvPV(ST(0), notused);
	user = SvPV(ST(1), notused);

	if (items > 2)
		reason = SvPV(ST(2), notused);

	if (chat_name == NULL || user == NULL)
		XSRETURN_IV(-1);

	if (reason == NULL)
		reason = "";

	if (items == 4) {
		u_int32_t acct_refnum = SvIV(ST(3));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	chat = chat_find(acct, chat_name);
	if (chat == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(chat_kick(acct, chat, user, reason));
}

XS(PORK_chat_topic) {
	char *chat_name;
	char *topic;
	size_t notused;
	struct pork_acct *acct;
	struct chatroom *chat;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	chat_name = SvPV(ST(0), notused);
	topic = SvPV(ST(1), notused);

	if (chat_name == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	chat = chat_find(acct, chat_name);
	if (chat == NULL)
		XSRETURN_IV(-1);

	if (topic == NULL || *topic == '\0') {
		if (chat->topic != NULL)
			XSRETURN_PV(chat->topic);
		else
			XSRETURN_UNDEF;
	}

	XSRETURN_IV(chat_set_topic(acct, chat, topic));
}

XS(PORK_chat_get_list) {
	dXSARGS;
	dlist_t *cur;
	struct pork_acct *acct;
	u_int32_t i = 0;

	(void) cv;

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	SP -= items;
	for (cur = acct->chat_list ; cur != NULL ; cur = cur->next) {
		struct chatroom *chat = cur->data;

		XPUSHs(sv_2mortal(newSVpv(chat->title, 0)));
		i++;
	}

	XSRETURN(i);
}

XS(PORK_chat_get_window) {
	char *chat_name;
	size_t notused;
	struct pork_acct *acct;
	struct imwindow *win;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	chat_name = SvPV(ST(0), notused);
	if (chat_name == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	win = imwindow_find_chat_target(acct, chat_name);
	if (win == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(win->refnum);
}

XS(PORK_chat_get_users) {
	char *chat_name;
	size_t notused;
	dlist_t *node;
	struct pork_acct *acct;
	struct chatroom *chat;
	u_int32_t flags = 0;
	u_int32_t args = 0;
	u_int32_t invert = 0;
	dXSARGS;

	(void) cv;

	if (items < 3)
		XSRETURN_EMPTY;

	chat_name = SvPV(ST(0), notused);
	if (chat_name == NULL)
		XSRETURN_EMPTY;

	flags = SvIV(ST(1));
	invert = SvIV(ST(2));

	if (items == 4) {
		u_int32_t acct_refnum = SvIV(ST(3));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_EMPTY;
	} else
		acct = cur_window()->owner;

	chat = chat_find(acct, chat_name);
	if (chat == NULL)
		XSRETURN_EMPTY;

	SP -= items;
	for (node = chat->user_list ; node != NULL ; node = node->next) {
		struct chat_user *user = node->data;

		if (!flags || (!!(user->status & flags)) ^ invert) {
			XPUSHs(sv_2mortal(newSVpv(user->nname, 0)));
			args++;
		}
	}

	XSRETURN(args);
}

XS(PORK_chat_ignore) {
	char *name;
	char *dest;
	size_t notused;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	name = SvPV(ST(0), notused);
	dest = SvPV(ST(1), notused);

	if (name == NULL || dest == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(chat_ignore(acct, name, dest));
}

XS(PORK_chat_invite) {
	char *name;
	char *dest;
	char *msg;
	size_t notused;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 3 && items != 4)
		XSRETURN_IV(-1);

	name = SvPV(ST(0), notused);
	dest = SvPV(ST(1), notused);
	msg = SvPV(ST(2), notused);

	if (name == NULL || dest == NULL)
		XSRETURN_IV(-1);

	if (items == 4) {
		u_int32_t acct_refnum = SvIV(ST(3));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	if (!acct->connected)
		XSRETURN_IV(-1);

	XSRETURN_IV(chat_invite(acct, name, dest, msg));
}

XS(PORK_chat_join) {
	char *chat_name;
	size_t notused;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	chat_name = SvPV(ST(0), notused);
	if (chat_name == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(chat_join(acct, chat_name));
}

XS(PORK_chat_leave) {
	char *chat_name;
	size_t notused;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	chat_name = SvPV(ST(0), notused);
	if (chat_name == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(chat_leave(acct, chat_name, 0));
}

XS(PORK_chat_send) {
	struct pork_acct *acct;
	char *msg;
	char *chat_name;
	size_t notused;
	struct chatroom *chat;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	chat_name = SvPV(ST(0), notused);
	msg = SvPV(ST(1), notused);

	if (chat_name == NULL || msg == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	chat = chat_find(acct, chat_name);
	if (chat == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(chat_send_msg(acct, chat, chat_name, msg));
}

XS(PORK_chat_unignore) {
	char *name;
	char *dest;
	size_t notused;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	name = SvPV(ST(0), notused);
	dest = SvPV(ST(1), notused);

	if (name == NULL || dest == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(chat_unignore(acct, name, dest));
}

XS(PORK_chat_target) {
	char *name = NULL;
	size_t notused;
	struct pork_acct *acct;
	struct chatroom *chat;
	dXSARGS;

	(void) cv;

	if (items > 0) {
		name = SvPV(ST(0), notused);
		if (name == NULL)
			XSRETURN_UNDEF;
	} else {
		if (cur_window()->type != WIN_TYPE_CHAT)
			XSRETURN_UNDEF;

		chat = cur_window()->data;
		XSRETURN_PV(chat->title);
	}

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_UNDEF;
	} else
		acct = cur_window()->owner;

	chat = chat_find(acct, name);
	if (chat == NULL)
		XSRETURN_UNDEF;

	XSRETURN_PV(chat->title);
}


