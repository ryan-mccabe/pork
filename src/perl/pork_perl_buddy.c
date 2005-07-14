/*
** pork_perl_buddy.c - Perl scripting support
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
#include <pork_buddy.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_perl_xs.h>
#include <pork_perl_macro.h>

XS(PORK_buddy_add) {
	size_t notused;
	char *target;
	char *group;
	struct pork_acct *acct;
	struct bgroup *bgroup;
	dXSARGS;

	if (items < 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	group = SvPV(ST(1), notused);

	if (target == NULL || group == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(2, IV(-1));
	bgroup = group_find(acct, group);
	if (group == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(buddy_add(acct, target, bgroup, 1) == NULL);
}

XS(PORK_buddy_add_block) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(buddy_add_block(acct, target, 1));
}

XS(PORK_buddy_clear_permit) {
	struct pork_acct *acct;
	dXSARGS;

	ACCT_WIN_REFNUM(0, IV(-1));
	buddy_clear_permit(acct);
	XSRETURN_IV(0);
}

XS(PORK_buddy_clear_block) {
	struct pork_acct *acct;
	dXSARGS;

	ACCT_WIN_REFNUM(0, IV(-1));
	buddy_clear_block(acct);
	XSRETURN_IV(0);
}

XS(PORK_buddy_add_permit) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(buddy_add_permit(acct, target, 1));
}

XS(PORK_buddy_alias) {
	size_t notused;
	char *target;
	char *alias;
	struct buddy *buddy;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	alias = SvPV(ST(1), notused);

	if (target == NULL || alias == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(2, IV(-1));
	buddy = buddy_find(acct, target);
	if (buddy == NULL)
		XSRETURN_IV(-1);

	buddy_alias(acct, buddy, alias, 0);
	XSRETURN_IV(0);
}

XS(PORK_buddy_get_alias) {
	size_t notused;
	char *target;
	struct buddy *buddy;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_EMPTY;

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_EMPTY;

	ACCT_WIN_REFNUM(1, EMPTY);
	buddy = buddy_find(acct, target);
	if (buddy == NULL)
		XSRETURN_EMPTY;

	XSRETURN_PV(buddy->name);
}

XS(PORK_buddy_remove_block) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(buddy_remove_block(acct, target, 1));
}

XS(PORK_buddy_get_groups) {
	dlist_t *cur;
	int args = 0;
	struct pork_acct *acct;
	dXSARGS;

	ACCT_WIN_REFNUM(0, EMPTY);
	if (acct->buddy_pref == NULL)
		XSRETURN_EMPTY;

	SP -= items;
	for (cur = acct->buddy_pref->group_list ; cur != NULL ; cur = cur->next) {
		struct bgroup *group = cur->data;

		XPUSHs(sv_2mortal(newSVpv(group->name, 0)));
		args++;
	}

	XSRETURN(args);
}

XS(PORK_buddy_get_block) {
	dlist_t *cur;
	int args = 0;
	struct pork_acct *acct;
	dXSARGS;

	ACCT_WIN_REFNUM(0, EMPTY);
	if (acct->buddy_pref == NULL)
		XSRETURN_EMPTY;

	SP -= items;
	for (cur = acct->buddy_pref->block_list ; cur != NULL ; cur = cur->next) {
		XPUSHs(sv_2mortal(newSVpv(cur->data, 0)));
		args++;
	}

	XSRETURN(args);
}

XS(PORK_buddy_get_permit) {
	dlist_t *cur;
	int args = 0;
	struct pork_acct *acct;
	dXSARGS;

	ACCT_WIN_REFNUM(0, EMPTY);
	if (acct->buddy_pref == NULL)
		XSRETURN_EMPTY;

	SP -= items;
	for (cur = acct->buddy_pref->permit_list ; cur != NULL ; cur = cur->next) {
		XPUSHs(sv_2mortal(newSVpv(cur->data, 0)));
		args++;
	}

	XSRETURN(args);
}

XS(PORK_buddy_get_group_members) {
	dlist_t *cur;
	char *group;
	struct bgroup *bgroup;
	size_t notused;
	int args = 0;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_EMPTY;

	group = SvPV(ST(0), notused);
	if (group == NULL)
		XSRETURN_EMPTY;

	ACCT_WIN_REFNUM(1, EMPTY);
	bgroup = group_find(acct, group);
	if (bgroup == NULL)
		XSRETURN_EMPTY;

	SP -= items;
	for (cur = bgroup->member_list ; cur != NULL ; cur = cur->next) {
		struct buddy *buddy = cur->data;

		XPUSHs(sv_2mortal(newSVpv(buddy->nname, 0)));
		args++;
	}

	XSRETURN(args);
}

XS(PORK_buddy_remove) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(buddy_remove(acct, target, 1));
}

XS(PORK_buddy_remove_group) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(group_remove(acct, target, 1));
}

XS(PORK_buddy_add_group) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	if (group_add(acct, target) == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(0);
}

XS(PORK_buddy_remove_permit) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	XSRETURN_IV(buddy_remove_permit(acct, target, 1));
}

XS(PORK_get_buddy_profile) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	if (!acct->connected)
		XSRETURN_IV(-1);

	XSRETURN_IV(acct->proto->get_profile(acct, target));
}

XS(PORK_get_buddy_away) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	if (items < 1)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	ACCT_WIN_REFNUM(1, IV(-1));
	if (!acct->connected)
		XSRETURN_IV(-1);

	XSRETURN_IV(acct->proto->get_away_msg(acct, target));
}
