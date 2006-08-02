/*
** pork_irc.h
** Copyright (C) 2003-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_IRC_H
#define __PORK_IRC_H

#define IRC_OUT_BUFLEN		2048
#define IRC_IN_BUFLEN		8192

#define IRC_CHAN_OP			0x01
#define IRC_CHAN_VOICE		0x02
#define IRC_CHAN_HALFOP		0x04

struct chatroom;

enum {
	MODE_PLUS = '+',
	MODE_MINUS = '-'
};

struct irc_session {
	int sock;
	pork_queue_t *inq;
	pork_queue_t *outq;

	char *servers[32];
	char *chanmodes;
	char *chantypes;
	char *prefix_types;
	char *prefix_codes;

	u_int32_t wallchops:1;
	u_int32_t excepts:1;
	u_int32_t capab:1;
	u_int32_t knock:1;
	u_int32_t invex:1;
	u_int32_t callerid:1;
	u_int32_t etrace:1;
	u_int32_t safelist:1;
	u_int32_t ssl:1;

	u_int32_t nick_len;
	u_int32_t kick_len;
	u_int32_t topic_len;
	u_int32_t num_servers;
	u_int32_t server_ssl;

	hash_t callbacks;

	time_t last_update;
	size_t input_offset;
	char input_buf[IRC_IN_BUFLEN];
	struct pref_val *prefs;
	void *data;
	void *transport;
	ssize_t (*sock_read)(void *, void *, size_t);
	ssize_t (*sock_write)(void *, const void *, size_t);
};

struct irc_chan_data {
	char mode_str[128];
	hash_t mode_args;
	/* This is such a stupid hack. */
	u_int32_t join_complete:1;
	u_int32_t joined:1;
};

struct irc_cmd_q {
	char *cmd;
	size_t len;
};

struct irc_chan_arg {
	int arg;
	char *val;
};

struct irc_input {
	char *tokens[20];
	char *args;
	char *cmd;
	char *orig;
	int numeric;
	u_int32_t num_tokens;
};

struct callback_handler {
	char *str;
	int (*handler)(struct pork_acct *acct, struct irc_input *in);
};

int irc_proto_init(struct pork_proto *proto);

int irc_flush_outq(struct irc_session *session);
int irc_connect(struct pork_acct *a, const char *server, int *sock);

int irc_send_raw(struct irc_session *session, char *str);
int irc_send_pong(struct irc_session *session, char *dest);
int irc_send_join(struct irc_session *session, char *channel, char *key);
int irc_send_login(struct irc_session *session);
int irc_send_privmsg(struct irc_session *session, char *dest, char *msg);
int irc_send_mode(struct irc_session *session, char *mode_str);
int irc_send_ctcp(struct irc_session *session, char *dest, char *msg);
int irc_send_ctcp_reply(struct irc_session *session, char *dest, char *msg);
int irc_send_names(struct irc_session *session, char *chan);
int irc_send_who(struct irc_session *session, char *dest);
int irc_send_whois(struct irc_session *session, char *dest);
int irc_send_whowas(struct irc_session *session, char *dest);
int irc_send_nick(struct irc_session *session, char *nick);
int irc_send_kick(struct irc_session *session, char *chan, char *nick, char *reason);
int irc_send_kill(struct irc_session *session, char *nick, char *msg);
int irc_send_part(struct irc_session *session, char *chan);
int irc_send_ping(struct irc_session *session, char *str);
int irc_send_quit(struct irc_session *session, char *reason);
int irc_send_topic(struct irc_session *session, char *chan, char *topic);
int irc_send_notice(struct irc_session *session, char *dest, char *msg);
int irc_kick(struct irc_session *session, char *chan, char *user, char *msg);
int irc_set_away(struct irc_session *session, char *msg);
int irc_send_action(struct irc_session *session, char *dest, char *msg);
int irc_chan_free(struct pork_acct *acct, void *data);
int irc_send_invite(struct irc_session *session, char *channel, char *user);

char *irc_get_chanmode_arg(struct irc_chan_data *chat, char mode);
int irc_chanmode_has_arg(struct irc_session *session, char mode);
int irc_input_dispatch(struct irc_session *session, u_int32_t flags);
char *irc_text_filter(char *str);
int irc_quote(struct irc_session *session, char *str);

int irc_callback_init(struct irc_session *session);
int irc_callback_clear(struct irc_session *session);
int irc_callback_add_defaults(struct irc_session *session);
int irc_callback_add(	struct irc_session *session,
						char *str,
						int (*handler)(struct pork_acct *, struct irc_input *));

#else
#	warning "included multiple times"
#endif
