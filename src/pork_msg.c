/*
** pork_msg.c
** Copyright (C) 2003-2006 Ryan McCabe <ryan@numb.org>
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
#include <time.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_misc.h>
#include <pork_buddy.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_acct_set.h>
#include <pork_format.h>
#include <pork_events.h>
#include <pork_imsg.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_screen_io.h>
#include <pork_screen.h>
#include <pork_msg.h>

int pork_msg_autoreply(struct pork_acct *acct, char *dest, char *msg) {
	struct imwindow *win;
	char buf[4096];
	int ret;

	if (acct->proto->send_msg_auto == NULL)
		return (-1);

	if (acct->proto->send_msg_auto(acct, dest, msg) == -1)
		return (-1);

	screen_get_query_window(acct, dest, &win);
	ret = fill_format_str(OPT_FORMAT_MSG_SEND_AUTO, buf, sizeof(buf), acct, dest, msg);
	if (ret < 1)
		return (-1);
	screen_print_str(win, buf, (size_t) ret, MSG_TYPE_PRIVMSG_SEND);
	imwindow_send_msg(win);
	return (0);
}

int pork_msg_send_auto(struct pork_acct *acct, char *sender) {
	dlist_t *node;
	u_int32_t hash_val;
	int ret = 0;

	if (acct->away_msg != NULL && acct->proto->send_msg_auto == NULL)
		return (-1);

	hash_val = string_hash(sender, acct->autoreply.order);
	node = hash_find(&acct->autoreply, sender, hash_val);
	if (node == NULL) {
		struct autoresp *autoresp;

		autoresp = xcalloc(1, sizeof(*autoresp));
		autoresp->name = xstrdup(sender);
		autoresp->last = time(NULL);

		hash_add(&acct->autoreply, autoresp, hash_val);
		ret = pork_msg_autoreply(acct, sender, acct->away_msg);
	} else {
		time_t time_now = time(NULL);
		struct autoresp *autoresp = node->data;

		/*
		** Only send someone an auto-reply every 10 minutes.
		** XXX: maybe this should be a configurable value.
		*/
		if (autoresp->last + 600 <= time_now) {
			autoresp->last = time_now;
			ret = pork_msg_autoreply(acct, sender, acct->away_msg);
		}
	}

	return (ret);
}

int pork_recv_msg(	struct pork_acct *acct,
					char *dest,
					char *sender,
					char *userhost,
					char *msg,
					int autoresp)
{
	struct imwindow *win;

	screen_get_query_window(acct, sender, &win);
	win->typing = 0;

	if (!event_generate(acct->events, EVENT_RECV_IM,
		sender, userhost, dest, autoresp, msg, acct->refnum))
	{
		int type;
		char buf[4096];
		int ret;

		if (autoresp)
			type = OPT_FORMAT_MSG_RECV_AUTO;
		else {
			if (win == globals.status_win)
				type = OPT_FORMAT_MSG_RECV_STATUS;
			else
				type = OPT_FORMAT_MSG_RECV;
		}

		ret = fill_format_str(type, buf, sizeof(buf), acct, dest,
				sender, userhost, msg);
		if (ret < 1)
			return (-1);
		screen_print_str(win, buf, (size_t) ret, MSG_TYPE_PRIVMSG_RECV);
		imwindow_recv_msg(win);

		if (acct->away_msg != NULL && !autoresp &&
			opt_get_bool(acct->prefs, ACCT_OPT_AUTOSEND_AWAY))
		{
			pork_msg_send_auto(acct, sender);
		}
	}

	return (0);
}

static void autoresp_destroy_cb(void *param __notused, void *data) {
	struct autoresp *autoresp = (struct autoresp *) data;

	free(autoresp->name);
	free(autoresp);
}

static int autoresp_compare_cb(void *l, void *r) {
	char *name = (char *) l;
	struct autoresp *autoresp = (struct autoresp *) r;

	return (strcasecmp(name, autoresp->name));
}

int pork_set_away(struct pork_acct *acct, char *msg) {
	if (msg == NULL)
		return (pork_set_back(acct));

	if (event_generate(acct->events, EVENT_SEND_AWAY, msg, acct->refnum))
		return (0);

	if (acct->away_msg != NULL) {
		free(acct->away_msg);
		hash_destroy(&acct->autoreply);
	}

	acct->away_msg = xstrdup(msg);
	hash_init(&acct->autoreply, 3, autoresp_compare_cb, autoresp_destroy_cb);

	if (acct->proto->set_away != NULL) {
		if (acct->proto->set_away(acct, msg) == -1) {
			screen_err_msg(_("An error occurred while setting %s away"),
				acct->username);

			return (-1);
		}
	}

	screen_win_msg(cur_window(), 1, 1, 0, MSG_TYPE_AWAY,
		_("%s is now away"), acct->username);
	return (0);
}

int pork_set_back(struct pork_acct *acct) {
	if (event_generate(acct->events, EVENT_SEND_AWAY, NULL, acct->refnum))
		return (0);

	if (acct->away_msg == NULL) {
		screen_err_msg(_("%s is not away"), acct->username);
		return (-1);
	}

	free(acct->away_msg);
	acct->away_msg = NULL;

	if (acct->proto->set_back != NULL) {
		if (acct->proto->set_back(acct) == -1) {
			screen_err_msg(_("An error occurred while setting %s unaway"),
				acct->username);

			return (-1);
		}
	}

	hash_destroy(&acct->autoreply);
	memset(&acct->autoreply, 0, sizeof(acct->autoreply));

	screen_win_msg(cur_window(), 1, 1, 0, MSG_TYPE_BACK,
		_("%s is no longer away"), acct->username);
	return (0);
}

int pork_msg_send(struct pork_acct *acct, char *dest, char *msg) {
	int ret = 0;

	if (!event_generate(acct->events, EVENT_SEND_IM, dest, msg, acct->refnum)) {
		if (acct->proto->send_msg != NULL) {
			ret = acct->proto->send_msg(acct, dest, msg);
			if (ret == -1) {
				screen_err_msg(_("Error: the last message to %s could not be sent"),
					dest);
			} else {
				struct imwindow *win;
				char buf[4096];
				int type;
				int ret;

				if (acct->away_msg != NULL) {
					if (opt_get_bool(acct->prefs, ACCT_OPT_SEND_REMOVES_AWAY))
						pork_set_back(acct);
				}

				if (screen_get_query_window(acct, dest, &win) != 0)
					screen_goto_window(win->refnum);

				if (win == globals.status_win)
					type = OPT_FORMAT_MSG_SEND_STATUS;
				else
					type = OPT_FORMAT_MSG_SEND;

				ret = fill_format_str(type, buf, sizeof(buf), acct, dest, msg);
				if (ret < 1)
					return (-1);
				screen_print_str(win, buf, (size_t) ret, MSG_TYPE_PRIVMSG_SEND);
				imwindow_send_msg(win);
			}
		}
	}

	return (ret);
}

int pork_set_profile(struct pork_acct *acct, char *profile) {
	int ret = 0;

	if (event_generate(acct->events, EVENT_SEND_PROFILE, profile, acct->refnum))
		return (1);

	free(acct->profile);
	if (profile == NULL)
		acct->profile = NULL;
	else
		acct->profile = xstrdup(profile);

	if (acct->proto->set_profile != NULL)
		ret = acct->proto->set_profile(acct, profile);

	if (ret == 0) {
		screen_win_msg(cur_window(), 1, 1, 0, MSG_TYPE_CMD_OUTPUT,
			_("Profile for %s was %s"), acct->username,
			(profile == NULL ? _("cleared") : _("set")));
	}

	return (ret);
}

int pork_set_idle(struct pork_acct *acct, u_int32_t seconds) {
	if (acct->proto->set_idle_time == NULL)
		return (-1);

	if (event_generate(acct->events, EVENT_SEND_IDLE, seconds, acct->refnum))
		return (-1);

	return (acct->proto->set_idle_time(acct, seconds));
}

int pork_set_idle_time(struct pork_acct *acct, u_int32_t seconds) {
	char timebuf[32];

	if (pork_set_idle(acct, seconds) == -1)
		return (0);

	time_to_str_full(seconds, timebuf, sizeof(timebuf));

	screen_win_msg(cur_window(), 1, 1, 0, MSG_TYPE_CMD_OUTPUT,
		_("%s's idle time set to %s"), acct->username, timebuf);

	return (0);
}

int pork_send_warn(struct pork_acct *acct, char *user) {
	int ret = 0;

	if (acct->proto->warn == NULL)
		return (-1);

	if (!event_generate(acct->events, EVENT_SEND_WARN, user, acct->refnum)) {
		ret = acct->proto->warn(acct, user);
		if (ret == 0) {
			struct imwindow *win;
			char buf[4096];

			win = imwindow_find(acct, user);
			if (win == NULL)
				win = cur_window();

			ret = fill_format_str(OPT_FORMAT_WARN_SEND, buf, sizeof(buf), acct->username, user);
			if (ret < 1)
				return (-1);
			screen_print_str(win, buf, (size_t) ret, MSG_TYPE_CMD_OUTPUT);
			imwindow_send_msg(win);
		}
	}

	return (ret);
}

int pork_recv_warn(struct pork_acct *acct, char *user, u_int16_t warn_level) {
	if (!event_generate(acct->events, EVENT_RECV_WARN, user, warn_level,
		acct->refnum))
	{
		int ret;
		char buf[4096];
		struct imwindow *win;

		win = imwindow_find(acct, user);
		if (win == NULL)
			win = globals.status_win;

		ret = fill_format_str(OPT_FORMAT_WARN_RECV, buf, sizeof(buf),
				acct->username, warn_level);

		if (ret < 1)
			return (-1);

		screen_print_str(win, buf, (size_t) ret, MSG_TYPE_STATUS);
		imwindow_send_msg(win);
		return (0);
	}

	return (0);
}

int pork_recv_warn_anon(struct pork_acct *acct, u_int16_t warn_level) {
	if (!event_generate(acct->events, EVENT_RECV_WARN_ANON, warn_level,
		acct->refnum))
	{
		int ret;
		char buf[4096];

		ret = fill_format_str(OPT_FORMAT_WARN_RECV_ANON, buf, sizeof(buf),
				acct->username, warn_level);

		if (ret < 1)
			return (-1);

		screen_print_str(globals.status_win, buf, (size_t) ret,
			MSG_TYPE_STATUS);
		imwindow_send_msg(globals.status_win);
		return (0);
	}

	return (0);
}

int pork_send_warn_anon(struct pork_acct *acct, char *user) {
	if (acct->proto->warn_anon == NULL)
		return (-1);

	if (!event_generate(acct->events, EVENT_SEND_WARN_ANON,
		user, acct->refnum))
	{
		int ret;

		ret = acct->proto->warn_anon(acct, user);
		if (ret == 0) {
			char buf[4096];
			struct imwindow *win;

			win = imwindow_find(acct, user);
			if (win == NULL)
				win = cur_window();

			ret = fill_format_str(OPT_FORMAT_WARN_SEND_ANON,
					buf, sizeof(buf), acct->username, user);

			if (ret < 1)
				return (-1);
			screen_print_str(win, buf, (size_t) ret, MSG_TYPE_CMD_OUTPUT);
			imwindow_send_msg(win);
		}
	}

	return (0);
}

int pork_change_nick(struct pork_acct *acct, char *nick) {
	if (acct->proto->change_nick != NULL)
		return (acct->proto->change_nick(acct, nick));

	return (-1);
}

int pork_recv_action(	struct pork_acct *acct,
						char *dest,
						char *sender,
						char *userhost,
						char *msg)
{
	if (!event_generate(acct->events, EVENT_RECV_ACTION,
		sender, userhost, dest, msg, acct->refnum))
	{
		struct imwindow *win;
		char buf[4096];
		int type;
		int ret;

		screen_get_query_window(acct, sender, &win);

		if (win == globals.status_win)
			type = OPT_FORMAT_ACTION_RECV_STATUS;
		else
			type = OPT_FORMAT_ACTION_RECV;

		ret = fill_format_str(type, buf, sizeof(buf), acct,
				dest, sender, userhost, msg);
		if (ret < 1)
			return (-1);
		screen_print_str(win, buf, (size_t) ret, MSG_TYPE_PRIVMSG_RECV);
		imwindow_recv_msg(win);
	}

	return (0);
}

int pork_action_send(struct pork_acct *acct, char *dest, char *msg) {
	if (acct->proto->send_action == NULL || dest == NULL)
		return (-1);

	if (event_generate(acct->events, EVENT_SEND_ACTION,
		dest, msg, acct->refnum))
	{
		return (0);
	}

	if (acct->proto->send_action(acct, dest, msg) != -1) {
		char buf[4096];
		struct imwindow *win;
		int type;
		int ret;

		screen_get_query_window(acct, dest, &win);

		if (win == globals.status_win)
			type = OPT_FORMAT_ACTION_SEND_STATUS;
		else
			type = OPT_FORMAT_ACTION_SEND;

		ret = fill_format_str(type, buf, sizeof(buf), acct, dest, msg);
		if (ret < 1)
			return (-1);
		screen_print_str(win, buf, (size_t) ret, MSG_TYPE_PRIVMSG_SEND);
		imwindow_send_msg(win);
	}

	return (0);
}

int pork_notice_send(struct pork_acct *acct, char *dest, char *msg) {
	struct imwindow *win;

	if (acct->proto->send_notice == NULL)
		return (-1);

	win = imwindow_find(acct, dest);
	if (win == NULL)
		win = cur_window();

	if (!event_generate(acct->events, EVENT_SEND_NOTICE,
		dest, msg, acct->refnum))
	{
		char buf[4096];
		int type;
		int ret;

		if (win == globals.status_win)
			type = OPT_FORMAT_NOTICE_SEND_STATUS;
		else
			type = OPT_FORMAT_NOTICE_SEND;

		ret = fill_format_str(type, buf, sizeof(buf), acct, dest, msg);
		if (ret < 1)
			return (-1);
		screen_print_str(win, buf, (size_t) ret, MSG_TYPE_NOTICE_SEND);
		imwindow_send_msg(win);
	}

	return (0);
}

int pork_recv_notice(	struct pork_acct *acct,
						char *dest,
						char *sender,
						char *userhost,
						char *msg)
{
	struct imwindow *win;
	int type;

	win = imwindow_find(acct, sender);
	if (win == NULL) {
		win = globals.status_win;
		type = OPT_FORMAT_NOTICE_RECV_STATUS;
	} else
		type = OPT_FORMAT_NOTICE_RECV;

	if (!event_generate(acct->events, EVENT_RECV_NOTICE,
		sender, userhost, dest, msg, acct->refnum))
	{
		char buf[4096];
		int ret;

		ret = fill_format_str(type, buf, sizeof(buf), acct,
				dest, sender, userhost, msg);
		if (ret < 1)
			return (-1);
		screen_print_str(win, buf, (size_t) ret, MSG_TYPE_NOTICE_RECV);
		imwindow_recv_msg(win);
	}

	return (0);
}

int pork_signoff(struct pork_acct *acct, char *msg) {
	if (acct->proto->signoff != NULL)
		return (acct->proto->signoff(acct, msg));

	return (0);
}
