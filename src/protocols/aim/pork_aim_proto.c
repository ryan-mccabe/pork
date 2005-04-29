/*
** pork_aim_proto.c - AIM OSCAR interface to pork.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This file is heavily based on src/protocols/oscar/oscar.c from gaim,
** which is more or less to say that chunks of code have been mostly
** copied from there. In turn, I think that oscar.c file was mostly lifted
** from faimtest. Any copyrights there apply here.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/types.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_imsg.h>
#include <pork_io.h>
#include <pork_misc.h>
#include <pork_color.h>
#include <pork_html.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_buddy.h>
#include <pork_chat.h>
#include <pork_transfer.h>
#include <pork_conf.h>
#include <pork_screen_io.h>

#include <libfaim/faimconfig.h>
#include <libfaim/aim.h>

#include <pork_aim.h>
#include <pork_aim_proto.h>

static int aim_buddy_add(struct pork_acct *acct, struct buddy *buddy) {
	int ret;
	struct aim_priv *priv = acct->data;
	aim_session_t *session = &priv->aim_session;

	if (session->ssi.received_data == 0) {
		debug("buddy add failed");
		return (-1);
	}

	if (aim_ssi_itemlist_exists(session->ssi.local, buddy->nname)) {
		debug("buddy %s already exists for %s", buddy->nname, acct->username);
		return (-1);
	}

	ret = aim_ssi_addbuddy(session, buddy->nname,
			buddy->group->name, buddy->name, NULL, NULL, 0);

	return (ret);
}

static int aim_buddy_alias(struct pork_acct *acct, struct buddy *buddy) {
	struct aim_priv *priv = acct->data;
	int ret;

	if (priv->aim_session.ssi.received_data == 0) {
		debug("buddy alias failed");
		return (-1);
	}

	ret = aim_ssi_aliasbuddy(&priv->aim_session,
			buddy->group->name, buddy->nname, buddy->name);

	return (ret);
}

static int aim_add_block(struct pork_acct *acct, char *target) {
	struct aim_priv *priv = acct->data;

	if (priv->aim_session.ssi.received_data == 0)
		return (-1);

	return (aim_ssi_adddeny(&priv->aim_session, target));
}

static int aim_add_permit(struct pork_acct *acct, char *target) {
	struct aim_priv *priv = acct->data;

	if (priv->aim_session.ssi.received_data == 0) {
		debug("add permit failed");
		return (-1);
	}

	return (aim_ssi_addpermit(&priv->aim_session, target));
}

static int aim_buddy_remove(struct pork_acct *acct, struct buddy *buddy) {
	struct aim_priv *priv = acct->data;
	int ret;

	if (priv->aim_session.ssi.received_data == 0) {
		debug("buddy remove failed");
		return (-1);
	}

	ret = aim_ssi_delbuddy(&priv->aim_session, buddy->nname,
			buddy->group->name);

	return (ret);
}

static int aim_remove_permit(struct pork_acct *acct, char *target) {
	struct aim_priv *priv = acct->data;

	if (priv->aim_session.ssi.received_data == 0) {
		debug("remove permit failed");
		return (-1);
	}

	return (aim_ssi_delpermit(&priv->aim_session, target));
}

static int aim_unblock(struct pork_acct *acct, char *target) {
	struct aim_priv *priv = acct->data;

	if (priv->aim_session.ssi.received_data == 0) {
		debug("unblock failed");
		return (-1);
	}

	return (aim_ssi_deldeny(&priv->aim_session, target));
}

static int aim_update_buddy(struct pork_acct *acct __notused,
							struct buddy *buddy,
							void *data)
{
	aim_userinfo_t *userinfo = data;

	buddy->signon_time = userinfo->onlinesince;
	buddy->warn_level = (float) userinfo->warnlevel / 10;

	if (userinfo->present & AIM_USERINFO_PRESENT_FLAGS)
		buddy->type = userinfo->flags;

	if (userinfo->present & AIM_USERINFO_PRESENT_SESSIONLEN)
		buddy->idle_time = userinfo->idletime;

	buddy->status = STATUS_ACTIVE;

	if (buddy->idle_time > 0)
		buddy->status = STATUS_IDLE;

	if (userinfo->flags & AIM_FLAG_AWAY)
		buddy->status = STATUS_AWAY;

	return (0);
}

static struct chatroom *aim_find_chat_name_data(struct pork_acct *acct,
												char *name)
{
	dlist_t *ret;

	ret = aim_find_chat_name(acct, name);
	if (ret == NULL || ret->data == NULL)
		return (NULL);

	return (ret->data);
}

static int aim_leave_chatroom(struct pork_acct *acct, struct chatroom *chat) {
	struct aim_priv *priv = acct->data;
	struct aim_chat *a_chat;

	a_chat = chat->data;

	pork_io_del(a_chat->conn);
	aim_conn_kill(&priv->aim_session, &a_chat->conn);

	a_chat->conn = NULL;
	return (0);
}

static int aim_join_chatroom(struct pork_acct *acct, char *name, char *args) {
	int ret;
	struct aim_priv *priv = acct->data;
	aim_conn_t *chatnav_conn;
	struct chatroom_info info;
	struct chatroom *chat;

	chat = chat_find(acct, name);
	if (chat != NULL && chat->data != NULL)
		return (0);

	if (aim_chat_parse_name(name, &info) != 0) {
		debug("aim_chat_parse_name failed for %s", name);
		return (-1);
	}

	chatnav_conn = priv->chatnav_conn;
	if (chatnav_conn == NULL) {
		struct chatroom_info *chat_info = xcalloc(1, sizeof(*chat_info));

		chat_info->name = xstrdup(info.name);
		chat_info->exchange = (u_int16_t) info.exchange;

		priv->chat_create_list = dlist_add_head(priv->chat_create_list,
									chat_info);
		aim_reqservice(&priv->aim_session, priv->bos_conn,
			AIM_CONN_TYPE_CHATNAV);

		free(info.name);
		return (0);
	}

	ret = aim_chatnav_createroom(&priv->aim_session, chatnav_conn,
			info.name, info.exchange);

	free(info.name);
	return (ret);
}

static int aim_chat_send(	struct pork_acct *acct,
					struct chatroom *chat,
					char *target __notused,
					char *msg)
{
	char *msg_html;
	int msg_len;
	int ret;
	struct aim_priv *priv = acct->data;
	struct aim_chat *a_chat;

	if (msg == NULL) {
		debug("aim_chat_send with NULL msg");
		return (-1);
	}

	a_chat = chat->data;

	msg_html = text_to_html(msg);
	msg_len = strlen(msg_html);

	if (msg_len > a_chat->max_msg_len) {
		debug("msg len too long");
		return (-1);
	}

	ret = aim_chat_send_im(&priv->aim_session, a_chat->conn,
			AIM_CHATFLAGS_NOREFLECT, msg_html, msg_len, "us-ascii", "en");

	return (ret);
}

static int aim_chat_action(struct pork_acct *acct,
					struct chatroom *chat,
					char *target __notused,
					char *msg)
{
	char buf[8192];
	int ret;

	if (!strncasecmp(msg, "<html>", 6))
		ret = snprintf(buf, sizeof(buf), "<HTML>/me %s", &msg[6]);
	else
		ret = snprintf(buf, sizeof(buf), "/me %s", msg);

	if (ret < 0 || (size_t) ret >= sizeof(buf))
		return (-1);

	return (aim_chat_send(acct, chat, chat->title_quoted, buf));
}

static int aim_chat_send_invite(struct pork_acct *acct,
								struct chatroom *chat,
								char *dest,
								char *msg)
{
	struct aim_priv *priv = acct->data;
	int ret;
	struct aim_chat *a_chat;

	a_chat = chat->data;

	if (msg == NULL)
		msg = "";

	ret = aim_im_sendch2_chatinvite(&priv->aim_session, dest, msg,
			a_chat->exchange, a_chat->fullname, 0);

	return (ret);
}

static int aim_search(struct pork_acct *acct, char *str) {
	struct aim_priv *priv = acct->data;

	return (aim_search_address(&priv->aim_session, priv->bos_conn, str));
}

static int aim_keepalive(struct pork_acct *acct) {
	struct aim_priv *priv = acct->data;

	return (aim_flap_nop(&priv->aim_session, priv->bos_conn));
}

static int aim_acct_update(struct pork_acct *acct) {
	struct aim_priv *priv;

	if (!acct->connected)
		return (-1);

	/*
	** We've got to keep track of AIM buddies' idle time
	** ourselves; the AIM server reports an idle time
	** for them only when there's a change (i.e. they
	** go idle, come back from idle or their client reports
	** a new idle time). This routine will run every minute
	** and increment the idle time of buddies where appropriate.
	*/

	priv = acct->data;
	if (time(NULL) >= priv->last_update + 60) {
		aim_keepalive(acct);
		buddy_update_idle(acct);
		aim_cleansnacs(&priv->aim_session, 59);
		time(&priv->last_update);
	}

	return (0);
}

static int aim_acct_init(struct pork_acct *acct) {
	char buf[NUSER_LEN];
	struct aim_priv *priv;

	normalize(buf, acct->username, sizeof(buf));

	/*
	** You can't have the same screen name logged in more than
	** once with AIM.
	*/
	if (pork_acct_find_name(buf, PROTO_AIM) != NULL)
		return (-1);

	free(acct->username);
	acct->username = xstrdup(buf);

	priv = xcalloc(1, sizeof(*priv));
	acct->data = priv;

	if (aim_setup(acct) != 0) {
		free(priv);
		return (-1);
	}

	if (acct->profile == NULL)
		acct->profile = xstrdup(DEFAULT_AIM_PROFILE);

	return (0);
}

static int aim_acct_free(struct pork_acct *acct) {
	aim_kill_all_conn(acct);
	free(acct->data);
	return (0);
}

static int aim_connect(struct pork_acct *acct, char *args) {
	if (acct->passwd == NULL) {
		if (args != NULL && !blank_str(args)) {
			acct->passwd = xstrdup(args);
			memset(args, 0, strlen(args));
		} else {
			char buf[128];

			screen_prompt_user("Password: ", buf, sizeof(buf));
			if (buf[0] == '\0') {
				screen_err_msg("There was an error reading your password");
				return (-1);
			}

			acct->passwd = xstrdup(buf);
			memset(buf, 0, sizeof(buf));
		}
	}

	return (aim_login(acct));
}

static int aim_read_config(struct pork_acct *acct) {
	return (read_user_config(acct));
}

static int aim_write_config(struct pork_acct *acct) {
	return (save_user_config(acct));
}

static int aim_file_recv_data(struct file_transfer *xfer, char *buf, size_t len) {
	struct aim_oft_info *oft_info = xfer->data;

	oft_info->fh.recvcsum = aim_oft_checksum_chunk(buf, len,
								oft_info->fh.recvcsum);

	if (xfer->bytes_sent + xfer->start_offset >= xfer->file_len)
		transfer_recv_complete(xfer);

	return (0);
}

static int aim_file_recv_complete(struct file_transfer *xfer) {
	struct aim_oft_info *oft_info = xfer->data;

	oft_info->fh.nrecvd = xfer->bytes_sent;

	aim_oft_sendheader(oft_info->sess, AIM_CB_OFT_DONE, oft_info);

	aim_clearhandlers(oft_info->conn);
	aim_conn_kill(oft_info->sess, &oft_info->conn);
	aim_oft_destroyinfo(oft_info);

	pork_io_del(xfer);
	return (0);
}

static int aim_file_send_complete(struct file_transfer *xfer) {
	struct aim_oft_info *oft_info = xfer->data;

	aim_clearhandlers(oft_info->conn);
	aim_conn_kill(oft_info->sess, &oft_info->conn);
	aim_oft_destroyinfo(oft_info);

	pork_io_del(xfer);
	return (0);
}

static int aim_file_send(struct file_transfer *xfer) {
	struct aim_priv *priv = xfer->acct->data;
	struct aim_oft_info *oft_info;

	if (transfer_bind_listen_sock(xfer, priv->bos_conn->fd) == -1)
		return (-1);

	oft_info = aim_oft_createinfo(&priv->aim_session, NULL,
				xfer->peer_username, xfer->laddr_ip, xfer->lport,
				xfer->file_len, 0, xfer->fname_base);

	oft_info->fh.checksum = aim_oft_checksum_file(xfer->fname_local);
	oft_info->port = xfer->lport;

	aim_sendfile_listen(&priv->aim_session, oft_info, xfer->sock);

	if (oft_info->conn == NULL) {
		aim_oft_destroyinfo(oft_info);
		return (-1);
	}

	oft_info->conn->priv = xfer;
	xfer->data = oft_info;

	pork_io_add(xfer->sock, IO_COND_RW, oft_info->conn, oft_info->conn,
		aim_listen_conn_event);

	aim_im_sendch2_sendfile_ask(&priv->aim_session, oft_info);
	aim_conn_addhandler(&priv->aim_session, oft_info->conn, AIM_CB_FAM_OFT, AIM_CB_OFT_ESTABLISHED, aim_file_send_accepted, 0);

	transfer_request_send(xfer);
	return (0);
}

static int aim_file_send_data(	struct file_transfer *xfer,
						char *buf __notused,
						size_t len __notused)
{
	if (xfer->status == TRANSFER_STATUS_COMPLETE) {
		pork_io_del(xfer);
		return (transfer_send_complete(xfer));
	}

	return (0);
}

static int aim_file_abort(struct file_transfer *xfer) {
	struct aim_oft_info *oft_info = xfer->data;

	pork_io_del(xfer);

	if (oft_info != NULL) {
		pork_io_del(oft_info->conn);
		aim_im_sendch2_sendfile_cancel(oft_info->sess, oft_info);
		aim_clearhandlers(oft_info->conn);

		if (!(xfer->protocol_flags & AIM_XFER_IN_HANDLER))
			aim_conn_kill(oft_info->sess, &oft_info->conn);
		else
			aim_conn_close(oft_info->conn);

		aim_oft_destroyinfo(oft_info);
	}

	return (0);
}

static int aim_file_accept(struct file_transfer *xfer) {
	struct aim_oft_info *oft_info;
	struct aim_priv *priv = xfer->acct->data;
	char buf[512];
	int ret;
	int sock;

	oft_info = xfer->data;

	snprintf(buf, sizeof(buf), "%s:%d", oft_info->verifiedip,
		oft_info->port);

	oft_info->conn = aim_newconn(&priv->aim_session,
						AIM_CONN_TYPE_RENDEZVOUS, NULL);

	if (oft_info->conn == NULL) {
		screen_err_msg("Error connecting to %s@%s while receiving %s",
			xfer->peer_username, buf, xfer->fname_local);
		return (-1);
	}

	oft_info->conn->subtype = AIM_CONN_SUBTYPE_OFT_SENDFILE;
	oft_info->conn->priv = xfer;

	ret = aim_sock_connect(buf, &xfer->acct->laddr, &sock);
	if (ret == 0) {
		aim_connected(sock, 0, oft_info->conn);
	} else if (ret == -EINPROGRESS) {
		oft_info->conn->status |= AIM_CONN_STATUS_INPROGRESS;
		pork_io_add(sock, IO_COND_WRITE, oft_info->conn, oft_info->conn,
			aim_connected);
	} else {
		aim_conn_kill(&priv->aim_session, &oft_info->conn);
		screen_err_msg("Error connecting to %s@%s while receiving %s",
			xfer->peer_username, buf, xfer->fname_local);
		return (-1);
	}

	aim_conn_addhandler(&priv->aim_session, oft_info->conn,
		AIM_CB_FAM_OFT, AIM_CB_OFT_PROMPT, aim_file_recv_accept, 0);
	aim_conn_addhandler(&priv->aim_session, oft_info->conn,
		AIM_CB_FAM_SPECIAL, AIM_CB_SPECIAL_CONNDEAD, aim_file_transfer_dead, 0);

	transfer_recv_accepted(xfer);
	return (0);
}

static char *aim_filter_text_out(char *msg) {
	if (!strncasecmp(msg, "<HTML", 5))
		return (strip_html(msg));

	return (color_quote_codes(msg));
}

static int aim_chat_get_name(	const char *str,
						char *buf,
						size_t len,
						char *arg_buf,
						size_t arg_len)
{
	struct chatroom_info info;
	int ret;

	if (aim_chat_parse_name(str, &info) != 0) {
		debug("aim_chat_parse_name failed for %s", str);
		return (-1);
	}

	ret = snprintf(buf, len, "%s/%d", info.name, info.exchange);
	free(info.name);

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int aim_set_privacy_mode(struct pork_acct *acct, int mode) {
	struct aim_priv *priv = acct->data;

	if (mode >= 0 && mode <= 5) {
		acct->buddy_pref->privacy_mode = mode;
		aim_ssi_setpermdeny(&priv->aim_session,
			acct->buddy_pref->privacy_mode, 0xffffffff);
	}

	return (acct->buddy_pref->privacy_mode);
}

static int aim_send_msg_auto(struct pork_acct *acct, char *dest, char *msg) {
	struct aim_priv *priv = acct->data;
	char *msg_html = text_to_html(msg);

	return (aim_im_sendch1(&priv->aim_session, dest, 1, msg_html));
}

static int aim_send_msg(struct pork_acct *acct, char *dest, char *msg) {
	struct aim_priv *priv = acct->data;
	char *msg_html = text_to_html(msg);

	return (aim_im_sendch1(&priv->aim_session, dest, 0, msg_html));
}

static int aim_action(struct pork_acct *acct, char *dest, char *msg) {
	char buf[8192];
	int ret;

	if (!strncasecmp(msg, "<html>", 6))
		ret = snprintf(buf, sizeof(buf), "<HTML>/me %s", &msg[6]);
	else
		ret = snprintf(buf, sizeof(buf), "/me %s", msg);

	if (ret < 0 || (size_t) ret >= sizeof(buf)) {
		debug("snprintf failed");
		return (-1);
	}

	return (aim_send_msg(acct, dest, buf));
}







static int aim_warn(struct pork_acct *acct, char *target) {
	struct aim_priv *priv = acct->data;

	return (aim_im_warn(&priv->aim_session, priv->bos_conn, target, 0));
}

static int aim_warn_anon(struct pork_acct *acct, char *target) {
	struct aim_priv *priv = acct->data;
	int ret;

	ret = aim_im_warn(&priv->aim_session, priv->bos_conn,
			target, AIM_WARN_ANON);

	return (ret);
}

static int aim_set_back(struct pork_acct *acct) {
	struct aim_priv *priv = acct->data;
	int ret;

	ret = aim_locate_setprofile(&priv->aim_session, NULL, NULL, 0, NULL, "", 0);
	return (ret);
}

static int aim_set_away(struct pork_acct *acct, char *away_msg) {
	size_t len;
	int ret;
	char *msg_html;
	struct aim_priv *priv = acct->data;

	msg_html = text_to_html(away_msg);

	len = strlen(msg_html);
	if (len > priv->rights.max_away_len) {
		screen_err_msg("%s's away message is too long. The maximum length is %u characters",
			acct->username, priv->rights.max_away_len);
		return (-1);
	}

	ret = aim_locate_setprofile(&priv->aim_session,
			NULL, NULL, 0, "us-ascii", msg_html, len);

	return (ret);
}

static int aim_set_idle(struct pork_acct *acct, u_int32_t idle_secs) {
	struct aim_priv *priv = acct->data;
	int ret;

	ret = aim_srv_setidle(&priv->aim_session, idle_secs);
	if (ret >= 0) {
		if (idle_secs > 0)
			acct->marked_idle = 1;
		else
			acct->marked_idle = 0;
	}

	return (ret);
}

static int aim_set_profile(struct pork_acct *acct, char *profile) {
	int ret;
	size_t len;
	char *profile_html;
	struct aim_priv *priv = acct->data;

	if (profile == NULL) {
		ret = aim_locate_setprofile(&priv->aim_session,
				NULL, "", 0, NULL, NULL, 0);

		return (ret);
	}

	profile_html = text_to_html(profile);

	len = strlen(profile_html);
	if (len > priv->rights.max_profile_len) {
		screen_err_msg("%s's profile is too long. The maximum length is %u characters",
			acct->username, priv->rights.max_profile_len);
		return (-1);
	}

	ret = aim_locate_setprofile(&priv->aim_session, "us-ascii",
			profile_html, len, NULL, NULL, 0);

	return (ret);
}

static int aim_get_away_msg(struct pork_acct *acct, char *buddy) {
	struct aim_priv *priv = acct->data;

	return (aim_locate_getinfoshort(&priv->aim_session, buddy, 0x00000002));
}

static int aim_whois(struct pork_acct *acct, char *buddy) {
	struct aim_priv *priv = acct->data;

	return (aim_locate_getinfoshort(&priv->aim_session, buddy, 0x00000003));
}

int aim_chat_print_users(	struct pork_acct *acct __notused,
							struct chatroom *chat)
{
	int ret;
	dlist_t *cur;
	char buf[2048];
	size_t i = 0;
	size_t len = sizeof(buf);
	struct chat_user *chat_user;
	struct aim_chat *a_chat;
	struct imwindow *win = chat->win;

	a_chat = chat->data;
	cur = chat->user_list;
	if (cur == NULL) {
		screen_win_msg(win, 0, 0, 1, MSG_TYPE_CHAT_STATUS,
			"%%D--%%m--%%M--%%x No %%Cu%%csers%%x in %%C%s%%x (%%W%s%%x)",
			chat->title_quoted, chat->title_full_quoted);

		return (0);
	}

	screen_win_msg(win, 0, 0, 1, MSG_TYPE_CHAT_STATUS,
		"%%D--%%m--%%M--%%x %u %%Cu%%csers%%x in %%C%s%%x (%%W%s%%x)",
		chat->num_users, chat->title_quoted, chat->title_full_quoted);

	while (cur != NULL) {
		chat_user = cur->data;

		if (chat_user->ignore)
			ret = snprintf(&buf[i], len, "[%%R%s%%x] ", chat_user->name);
		else
			ret = snprintf(&buf[i], len, "[%%B%s%%x] ", chat_user->name);

		if (ret < 0 || (size_t) ret >= len) {
			screen_err_msg("The results were too long to display");
			return (0);
		}

		len -= ret;
		i += ret;

		cur = cur->next;
	}

	if (i > 0 && buf[i - 1] == ' ')
		buf[--i] = '\0';

	screen_win_msg(win, 0, 0, 1, MSG_TYPE_CHAT_STATUS, "%s", buf);
	return (0);
}

int aim_chat_free(struct pork_acct *acct, void *data) {
	struct aim_chat *a_chat = data;

	if (a_chat->conn != NULL) {
		struct aim_priv *priv = acct->data;

		pork_io_del(a_chat->conn);
		aim_conn_kill(&priv->aim_session, &a_chat->conn);
	}

	free(a_chat->fullname);
	free(a_chat->fullname_quoted);
	free(a_chat->title);
	free(a_chat);

	return (0);
}

int aim_report_idle(struct pork_acct *acct, int mode) {
	struct aim_priv *priv = acct->data;

	u_int32_t report_idle = aim_ssi_getpresence(priv->aim_session.ssi.local);
	int ret;

	if (mode != 0)
		report_idle |= 0x0000400;
	else
		report_idle &= ~0x0000400;

	acct->report_idle = ((report_idle & 0x0000400) != 0);
	ret = aim_ssi_setpresence(&priv->aim_session, report_idle);
	return (ret);
}

int aim_connect_abort(struct pork_acct *acct) {
	aim_kill_all_conn(acct);
	aim_setup(acct);

	return (0);
}

int aim_proto_init(struct pork_proto *proto) {
	proto->buddy_add = aim_buddy_add;
	proto->buddy_alias = aim_buddy_alias;
	proto->buddy_block = aim_add_block;
	proto->buddy_permit = aim_add_permit;
	proto->buddy_remove = aim_buddy_remove;
	proto->buddy_remove_permit = aim_remove_permit;
	proto->buddy_unblock = aim_unblock;
	proto->buddy_update = aim_update_buddy;

	proto->chat_free = aim_chat_free;
	proto->chat_find = aim_find_chat_name_data;
	proto->chat_action = aim_chat_action;
	proto->chat_invite = aim_chat_send_invite;
	proto->chat_join = aim_join_chatroom;
	proto->chat_leave = aim_leave_chatroom;
	proto->chat_name = aim_chat_get_name;
	proto->chat_send = aim_chat_send;
	proto->chat_users = aim_chat_print_users;
	proto->chat_who = aim_chat_print_users;

	proto->who = aim_search;
	proto->connect = aim_connect;
	proto->connect_abort = aim_connect_abort;
	proto->reconnect = aim_connect;
	proto->free = aim_acct_free;
	proto->get_away_msg = aim_get_away_msg;
	proto->get_profile = aim_whois;
	proto->init = aim_acct_init;
	proto->keepalive = aim_keepalive;
	proto->filter_text = strip_html;
	proto->filter_text_out = aim_filter_text_out;
	proto->normalize = normalize;
	proto->user_compare = aim_sncmp;
	proto->read_config = aim_read_config;
	proto->send_msg = aim_send_msg;
	proto->send_msg_auto = aim_send_msg_auto;
	proto->set_away = aim_set_away;
	proto->set_back = aim_set_back;
	proto->set_idle_time = aim_set_idle;
	proto->set_privacy_mode = aim_set_privacy_mode;
	proto->set_profile = aim_set_profile;
	proto->set_report_idle = aim_report_idle;
	proto->update = aim_acct_update;
	proto->warn = aim_warn;
	proto->send_action = aim_action;
	proto->warn_anon = aim_warn_anon;
	proto->write_config = aim_write_config;

	proto->file_send = aim_file_send;
	proto->file_send_data = aim_file_send_data;
	proto->file_accept = aim_file_accept;
	proto->file_abort = aim_file_abort;
	proto->file_recv_data = aim_file_recv_data;
	proto->file_recv_complete = aim_file_recv_complete;
	proto->file_send_complete = aim_file_send_complete;

	return (0);
}
