
/*
** pork_aim_proto.h - AIM OSCAR protocol interface to pork.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_AIM_PROTO_H
#define __PORK_AIM_PROTO_H

int aim_proto_init(struct pork_proto *proto);
int aim_chat_free(struct pork_acct *acct, void *data);
int aim_connect_abort(struct pork_acct *acct);
int aim_report_idle(struct pork_acct *acct, int mode);
int aim_login(struct pork_acct *acct);
int aim_search(struct pork_acct *acct, char *str);
int aim_set_privacy_mode(struct pork_acct *acct, int mode);

int aim_chat_print_users(	struct pork_acct *acct __notused,
							struct chatroom *chat);

#endif
