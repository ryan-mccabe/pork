/*
** pork_aim.h - pork's interface with libfaim
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_AIM_H
#define __PORK_AIM_H

#define AIM_DEFAULT_CHAT_EXCHANGE	4
#define AIM_XFER_IN_HANDLER			1

#define DEFAULT_AIM_PROFILE "<HTML>i &lt;3 <a href=\"http://dev.ojnk.net\">pork</a>"

#define FAIM_CB(x) int x(aim_session_t *session __notused, aim_frame_t *fr __notused, ...)

struct buddy;
struct bgroup;

enum {
	AIM_BUDDY_TYPE_UNCONFIRMED,
	AIM_BUDDY_TYPE_NORMAL,
	AIM_BUDDY_TYPE_AOL,
	AIM_BUDDY_TYPE_ADMIN,
	AIM_BUDDY_TYPE_ACTIVEBUDDY,
	AIM_BUDDY_TYPE_WIRELESS,
};

struct aim_chat {
	char *title;
	char *fullname;
	char *fullname_quoted;
	char *description;
	u_int16_t exchange;
	u_int16_t instance;
	aim_conn_t *conn;
	int max_msg_len;
	int max_visible_len;
	time_t created;
};

struct chatroom_info {
	char *name;
	u_int16_t exchange;
};

struct aim_priv {
	u_int32_t marked_idle:1;
	time_t last_update;

	aim_session_t aim_session;

	struct aim_rights {
		u_int32_t max_buddies;
		u_int32_t max_groups;
		u_int32_t max_watchers;
		u_int32_t max_permit;
		u_int32_t max_deny;
		u_int32_t max_away_len;
		u_int32_t max_profile_len;
	} rights;

	aim_conn_t *bos_conn;
	aim_conn_t *chatnav_conn;
	dlist_t *chat_create_list;
};

int aim_kill_all_conn(struct pork_acct *acct);
int aim_chat_parse_name(const char *name, struct chatroom_info *info);
dlist_t *aim_find_chat_name(struct pork_acct *acct, char *name);
void aim_connected(int sock, u_int32_t cond __notused, void *data);
void aim_listen_conn_event(int sock, u_int32_t cond, void *data);
int aim_setup(struct pork_acct *acct);
int aim_sock_connect(	const char *ip,
						struct sockaddr_storage *laddr,
						int *sock);

FAIM_CB(aim_file_send_accepted);
FAIM_CB(aim_file_transfer_dead);
FAIM_CB(aim_file_recv_accept);

#endif
