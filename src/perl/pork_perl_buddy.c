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
#include <pork_imwindow.h>
#include <pork_acct.h>
#include <pork_screen.h>
#include <pork_perl_xs.h>

XS(PORK_buddy_add) {
	size_t notused;
	char *target;
	char *group;
	struct pork_acct *acct;
	struct bgroup *bgroup;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	group = SvPV(ST(1), notused);

	if (target == NULL || group == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

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

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(buddy_add_block(acct, target, 1));
}

XS(PORK_buddy_clear_permit) {
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	buddy_clear_permit(acct);
	XSRETURN_IV(0);
}

XS(PORK_buddy_clear_block) {
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 0 && items != 1)
		XSRETURN_IV(-1);

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	buddy_clear_block(acct);
	XSRETURN_IV(0);
}

XS(PORK_buddy_add_permit) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(buddy_add_permit(acct, target, 1));
}

XS(PORK_buddy_alias) {
	size_t notused;
	char *target;
	char *alias;
	struct buddy *buddy;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 2 && items != 3)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	alias = SvPV(ST(1), notused);

	if (target == NULL || alias == NULL)
		XSRETURN_IV(-1);

	if (items == 3) {
		u_int32_t acct_refnum = SvIV(ST(2));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

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

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_EMPTY;

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_EMPTY;

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_EMPTY;
	} else
		acct = cur_window()->owner;

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

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(buddy_remove_block(acct, target, 1));
}

XS(PORK_buddy_get_groups) {
	dlist_t *cur;
	int args = 0;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_EMPTY;
	} else
		acct = cur_window()->owner;

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

	(void) cv;

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_EMPTY;
	} else
		acct = cur_window()->owner;

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

	(void) cv;

	if (items == 1) {
		u_int32_t acct_refnum = SvIV(ST(0));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_EMPTY;
	} else
		acct = cur_window()->owner;

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

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_EMPTY;

	group = SvPV(ST(0), notused);
	if (group == NULL)
		XSRETURN_EMPTY;

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_EMPTY;
	} else
		acct = cur_window()->owner;

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

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(buddy_remove(acct, target, 1));
}

XS(PORK_buddy_remove_group) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(group_remove(acct, target, 1));
}

XS(PORK_buddy_add_group) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	if (group_add(acct, target) == NULL)
		XSRETURN_IV(-1);

	XSRETURN_IV(0);
}

XS(PORK_buddy_remove_permit) {
	size_t notused;
	char *target;
	struct pork_acct *acct;
	dXSARGS;

	(void) cv;

	if (items != 1 && items != 2)
		XSRETURN_IV(-1);

	target = SvPV(ST(0), notused);
	if (target == NULL)
		XSRETURN_IV(-1);

	if (items == 2) {
		u_int32_t acct_refnum = SvIV(ST(1));

		acct = pork_acct_get_data(acct_refnum);
		if (acct == NULL)
			XSRETURN_IV(-1);
	} else
		acct = cur_window()->owner;

	XSRETURN_IV(buddy_remove_permit(acct, target, 1));
}
