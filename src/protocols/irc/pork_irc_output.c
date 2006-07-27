/*
** pork_irc_output.c
** Copyright (C) 2003-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <errno.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_queue.h>
#include <pork_inet.h>
#include <pork_io.h>
#include <pork_acct.h>
#include <pork_opt.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_chat.h>
#include <pork_missing.h>

#include <pork_irc.h>

static int irc_send_server(	struct irc_session *session,
							char *cmd,
							size_t len)
{
	return (sock_write(session->transport, cmd, len, session->sock_write));
}

static int irc_send(struct irc_session *session, char *command, size_t len) {
	int ret;

	if (session->sock < 0) {
		struct irc_cmd_q *cmd = xmalloc(sizeof(*cmd));

		cmd->cmd = xstrdup(command);
		cmd->len = len;

		if (queue_add(session->outq, cmd) != 0) {
			screen_err_msg(_("Error: %s: Error adding IRC command to the outbound queue."),
				((struct pork_acct *) session->data)->username);

			free(cmd->cmd);
			free(cmd);
			return (-1);
		}

		return (0);
	}

	ret = irc_send_server(session, command, len);
	if (ret == -1)
		pork_sock_err(session->data, session->sock);

	return (ret);
}

int irc_flush_outq(struct irc_session *session) {
	struct irc_cmd_q *cmd;
	int ret = 0;

	while ((cmd = queue_get(session->outq)) != NULL) {
		if (irc_send_server(session, cmd->cmd, cmd->len) > 0) {
			ret++;
			free(cmd->cmd);
			free(cmd);
		} else {
			debug("adding %s back to the queue", cmd->cmd);
			queue_putback_head(session->outq, cmd);
		}
	}

	return (ret);
}

int irc_connect(struct pork_acct *acct,
				const char *server,
				int *sock)
{
	struct sockaddr_storage ss;
	struct sockaddr_storage local;
	in_port_t port_num;
	char *irchost = getenv("IRCHOST");
	char *port;
	char buf[IRC_OUT_BUFLEN];
	char *passwd = NULL;

	if (server == NULL || xstrncpy(buf, server, sizeof(buf)) == -1)
		return (-1);

	memset(&ss, 0, sizeof(ss));
	memset(&local, 0, sizeof(local));

	port = strchr(buf, ':');
	if (port != NULL) {
		*port++ = '\0';

		passwd = strchr(port, ':');
		if (passwd != NULL) {
			*passwd++ = '\0';
		}
	} else
		port = "6667"; /* XXX FIXME */

	if (get_port(port, &port_num) != 0) {
		screen_err_msg(_("Error: %s: Invalid IRC server port: %s"),
			acct->username, port);
		memset(buf, 0, sizeof(buf));
		return (-1);
	}

	if (get_addr(buf, &ss) != 0) {
		screen_err_msg(_("Error: %s: Invalid IRC server host: %s"),
			acct->username, buf);
		memset(buf, 0, sizeof(buf));
		return (-1);
	}

	if (irchost != NULL) {
		if (get_addr(irchost, &local) != 0) {
			screen_err_msg(_("Error: %s: Invalid local hostname: %s"),
				acct->username, irchost);
			memcpy(&local, &acct->laddr, sizeof(local));
		}
	} else
		memcpy(&local, &acct->laddr, sizeof(local));

	free(acct->fport);
	acct->fport = xstrdup(port);

	free(acct->server);
	acct->server = xstrdup(buf);

	if (passwd != NULL && passwd[0] != '\0') {
		free_str_wipe(acct->passwd);
		acct->passwd = xstrdup(passwd);
	}

	sin_set_port(&local, acct->lport);
	memset(buf, 0, sizeof(buf));
	return (nb_connect(&ss, &local, port_num, sock));
}

int irc_send_raw(struct irc_session *session, char *str) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	ret = snprintf(buf, sizeof(buf), "%s\r\n", str);
	if (ret < 1 || (size_t) ret >= sizeof(buf))
		return (-1);
	return (irc_send(session, buf, ret));
}

int irc_send_mode(struct irc_session *session, char *mode_str) {
	int ret;
	char buf[IRC_OUT_BUFLEN];

	ret = snprintf(buf, sizeof(buf), "MODE %s\r\n", mode_str);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_pong(struct irc_session *session, char *dest) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	ret = snprintf(buf, sizeof(buf), "PONG %s\r\n", dest);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_set_away(struct irc_session *session, char *msg) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (msg != NULL)
		ret = snprintf(buf, sizeof(buf), "AWAY :%s\r\n", msg);
	else
		ret = snprintf(buf, sizeof(buf), "AWAY\r\n");

	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_invite(struct irc_session *session, char *channel, char *user) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	ret = snprintf(buf, sizeof(buf), "INVITE %s %s\r\n", user, channel);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_join(struct irc_session *session, char *channel, char *key) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (key != NULL)
		ret = snprintf(buf, sizeof(buf), "JOIN %s %s\r\n", channel, key);
	else
		ret = snprintf(buf, sizeof(buf), "JOIN %s\r\n", channel);

	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_login(struct irc_session *session) {
	char buf[IRC_OUT_BUFLEN];
	struct pork_acct *acct = session->data;
	int ret;

	if (acct->passwd != NULL) {
		ret = snprintf(buf, sizeof(buf), "PASS %s\r\n", acct->passwd);
		if (ret < 0 || (size_t) ret >= sizeof(buf))
			return (-1);

		ret = irc_send(session, buf, ret);
		if (ret == -1)
			return (-1);
	}

	ret = snprintf(buf, sizeof(buf), "NICK %s\r\n", acct->username);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	ret = irc_send(session, buf, ret);
	if (ret == -1)
		return (-1);

	ret = snprintf(buf, sizeof(buf), "USER %s ojnk ojnk :%s\r\n",
			acct->username, acct->profile);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_privmsg(struct irc_session *session, char *dest, char *msg) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	ret = snprintf(buf, sizeof(buf), "PRIVMSG %s :%s\r\n", dest, msg);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_ctcp(struct irc_session *session, char *dest, char *msg) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (!strcmp(dest, "*"))
		dest = cur_window()->target;

	ret = snprintf(buf, sizeof(buf), "PRIVMSG %s :\x01%s\x01\r\n", dest, msg);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_ping(struct irc_session *session, char *str) {
	char buf[IRC_OUT_BUFLEN];
	int ret;
	struct timeval tv;

	gettimeofday(&tv, NULL);

	ret = snprintf(buf, sizeof(buf), "PING %ld %ld", tv.tv_sec, tv.tv_usec);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send_ctcp(session, str, buf));
}

int irc_send_ctcp_reply(struct irc_session *session, char *dest, char *msg) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	ret = snprintf(buf, sizeof(buf), "NOTICE %s :\x01%s\x01\r\n", dest, msg);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_names(struct irc_session *session, char *chan) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (chan != NULL)
		ret = snprintf(buf, sizeof(buf), "NAMES :%s\r\n", chan);
	else
		ret = snprintf(buf, sizeof(buf), "NAMES\r\n");

	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_who(struct irc_session *session, char *dest) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (dest != NULL)
		ret = snprintf(buf, sizeof(buf), "WHO :%s\r\n", dest);
	else
		ret = snprintf(buf, sizeof(buf), "WHO\r\n");

	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_whois(struct irc_session *session, char *dest) {
	char buf[IRC_OUT_BUFLEN];
	int ret;
	struct pork_acct *acct = session->data;

	if (dest == NULL)
		dest = acct->username;

	ret = snprintf(buf, sizeof(buf), "WHOIS %s\r\n", dest);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_whowas(struct irc_session *session, char *dest) {
	char buf[IRC_OUT_BUFLEN];
	int ret;
	struct pork_acct *acct = session->data;

	if (dest == NULL)
		dest = acct->username;

	ret = snprintf(buf, sizeof(buf), "WHOWAS %s\r\n", dest);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_nick(struct irc_session *session, char *nick) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (nick == NULL)
		return (-1);

	ret = snprintf(buf, sizeof(buf), "NICK :%s\r\n", nick);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_part(struct irc_session *session, char *chan) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (chan == NULL)
		return (-1);

	ret = snprintf(buf, sizeof(buf), "PART %s\r\n", chan);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_quit(struct irc_session *session, char *reason) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (reason == NULL)
		reason = "";

	ret = snprintf(buf, sizeof(buf), "QUIT :%s\r\n", reason);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	pork_io_del(session);
	return (irc_send(session, buf, ret));
}

int irc_send_notice(struct irc_session *session, char *dest, char *msg) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	ret = snprintf(buf, sizeof(buf), "NOTICE %s :%s\r\n", dest, msg);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_kick(	struct irc_session *session,
					char *chan,
					char *nick,
					char *msg)
{
	char buf[IRC_OUT_BUFLEN];
	int ret;

	ret = snprintf(buf, sizeof(buf), "KICK %s %s :%s\r\n", chan, nick, msg);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_kill(struct irc_session *session, char *nick, char *msg) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	ret = snprintf(buf, sizeof(buf), "KILL %s :%s\r\n", nick, msg);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_topic(struct irc_session *session, char *chan, char *topic) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (topic != NULL)
		ret = snprintf(buf, sizeof(buf), "TOPIC %s :%s\r\n", chan, topic);
	else
		ret = snprintf(buf, sizeof(buf), "TOPIC %s\r\n", chan);

	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send(session, buf, ret));
}

int irc_send_action(struct irc_session *session, char *dest, char *msg) {
	char buf[IRC_OUT_BUFLEN];
	int ret;

	if (dest == NULL || msg == NULL)
		return (-1);

	ret = snprintf(buf, sizeof(buf), "ACTION %s", msg);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (irc_send_ctcp(session, dest, buf));
}
