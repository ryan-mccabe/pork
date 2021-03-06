/*
** pork_format_global.c - Facilities for filling global format strings.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_color.h>
#include <pork_buddy.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_slist.h>
#include <pork_buddy_list.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_proto.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_transfer.h>
#include <pork_chat.h>
#include <pork_format.h>

static int format_status_timestamp(	char opt,
									char *buf,
									size_t len,
									va_list ap __notused)
{
	time_t cur_time;
	struct tm *tm;
	int ret = 0;

	cur_time = time(NULL);
	tm = localtime(&cur_time);

	if (tm == NULL) {
		debug("localtime: %s", strerror(errno));
		return (-1);
	}

	switch (opt) {
		/* Hours, 24-hour format */
		case 'H':
			ret = snprintf(buf, len, "%02d", tm->tm_hour);
			break;

		/* Hours, 12-hour format */
		case 'h': {
			u_int32_t time_12hour;

			if (tm->tm_hour == 0)
				time_12hour = 12;
			else {
				time_12hour = tm->tm_hour;

				if (time_12hour > 12)
					time_12hour -= 12;
			}

			ret = snprintf(buf, len, "%02d", time_12hour);
			break;
		}

		/* Minutes */
		case 'M':
		case 'm':
			ret = snprintf(buf, len, "%02d", tm->tm_min);
			break;

		/* Seconds */
		case 'S':
		case 's':
			ret = snprintf(buf, len, "%02d", tm->tm_sec);
			break;

		/* AM or PM */
		case 'Z':
		case 'z':
			ret = xstrncpy(buf, (tm->tm_hour >= 12 ? "pm" : "am"), len);
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_status_activity(	char opt,
									char *buf,
									size_t len,
									va_list ap __notused)
{
	switch (opt) {
		/* Activity display */
		case 'A':
		case 'a': {
			dlist_t *cur = globals.window_list;
			size_t n = 0;

			do {
				struct imwindow *imwindow = cur->data;

				if (imwindow->swindow.activity &&
					!imwindow->ignore_activity &&
					!imwindow->swindow.visible)
				{
					int ret = snprintf(&buf[n], len - n, "%u,",
								imwindow->refnum);

					if (ret < 0 || (size_t) ret >= len - n)
						return (-1);

					n += ret;
				}

				cur = cur->next;
			} while (cur != globals.window_list);

			if (n > 0)
				buf[n - 1] = '\0';
			else
				return (1);

			break;
		}

		default:
			return (-1);
	}

	return (0);
}

static int format_status_typing(char opt, char *buf, size_t len, va_list ap) {
	int ret = 0;
	struct imwindow *imwindow = va_arg(ap, struct imwindow *);

	switch (opt) {
		/* Typing status */
		case 'Y':
			if (imwindow->typing) {
				char *str;

				if (imwindow->typing == 1)
					str = opt_get_str(globals.prefs, OPT_TEXT_TYPING_PAUSED);
				else
					str = opt_get_str(globals.prefs, OPT_TEXT_TYPING);

				if (xstrncpy(buf, str, len) == -1)
					return (-1);
			} else
				return (1);

			break;

		default:
			return (-1);
	}

	return (ret);
}

static int format_status_held(char opt, char *buf, size_t len, va_list ap) {
	struct imwindow *imwindow = va_arg(ap, struct imwindow *);
	int ret = 0;

	if (imwindow->swindow.held == 0)
		return (1);

	switch (opt) {
		/* Held indicator */
		case 'H':
			ret = snprintf(buf, len, "%02d", imwindow->swindow.held);
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_status_idle(char opt, char *buf, size_t len, va_list ap) {
	struct pork_acct *acct = va_arg(ap, struct pork_acct *);
	int hide_if_zero = va_arg(ap, int);
	int ret = 0;

	switch (opt) {
		/* Idle time */
		case 'I':
		own:
			if (acct->idle_time == 0 && hide_if_zero)
				return (1);

			ret = time_to_str(acct->idle_time, buf, len);
			break;

		/*
		** If the current window is a chat window, $i will display the
		** idle time of the user we're talking to.
		*/
		case 'i': {
			struct imwindow *win = cur_window();

			if (win->type == WIN_TYPE_PRIVMSG) {
				struct buddy *buddy;

				buddy = buddy_find(acct, win->target);
				if (buddy == NULL || (buddy->idle_time == 0 && hide_if_zero))
					return (1);

				ret = time_to_str(buddy->idle_time, buf, len);
				break;
			}

			goto own;
		}

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_status(char opt, char *buf, size_t len, va_list ap) {
	struct imwindow *imwindow = va_arg(ap, struct imwindow *);
	struct pork_acct *acct = va_arg(ap, struct pork_acct *);
	int ret = 0;

	switch (opt) {
		/* Disconnect / reconnect status */
		case 'D':
			if (acct->reconnecting)
				ret = xstrncpy(buf, _("reconnecting"), len);
			else if (acct->disconnected)
				ret = xstrncpy(buf, _("disconnected"), len);
			else
				ret = xstrncpy(buf, _("connected"), len);
			break;

		/* Screen name */
		case 'n':
		case 'N':
			ret = xstrncpy(buf, acct->username, len);
			break;

		/* Remote server */
		case 'S':
		case 's':
			if (acct->server != NULL)
				ret = xstrncpy(buf, acct->server, len);
			break;

		/* Remote port */
		case 'P':
		case 'p':
			if (acct->fport != NULL)
				ret = xstrncpy(buf, acct->fport, len);
			break;

		/* Window name */
		case 'z':
		case 'Z':
			ret = xstrncpy(buf, imwindow->name, len);
			break;

		/* Chat room, if inactive */
		case 'c':
		case 'C':
			if (imwindow->type == WIN_TYPE_CHAT &&
				(imwindow->data == NULL || !acct->connected))
			{
				ret = xstrncpy(buf,
						opt_get_str(globals.prefs, OPT_TEXT_NO_ROOM),
						len);
			}
			break;

		/* Chat mode, if applicable; M includes arguments, m doesn't. */
		case 'M':
		case 'm':
			if (imwindow->type == WIN_TYPE_CHAT && imwindow->data != NULL) {
				struct chatroom *chat = imwindow->data;

				ret = xstrncpy(buf, chat->mode, len);
				if (opt == 'm') {
					char *p;

					p = strchr(buf, ' ');
					if (p != NULL)
						*p = '\0';
				}
			}
			break;

		/* Chat status, if applicable */
		case '@':
			if (imwindow->type == WIN_TYPE_CHAT && imwindow->data != NULL) {
				struct chatroom *chat = imwindow->data;
				struct chat_user *user;

				user = chat_find_user(acct, chat, acct->username);
				if (user == NULL)
					break;

				if (user->status & CHAT_STATUS_OP)
					ret = xstrncpy(buf, "@", len);
				else if (user->status & CHAT_STATUS_HALFOP)
					ret = xstrncpy(buf, "%%", len);
				else if (user->status & CHAT_STATUS_VOICE)
					ret = xstrncpy(buf, "+", len);
			}
			break;

		/* User status */
		case '!':
			if (acct->disconnected) {
				if (acct->reconnecting)
					ret = xstrncpy(buf, _("reconnecting"), len);
				else
					ret = xstrncpy(buf, _("disconnected"), len);
			} else if (acct->connected) {
				if (acct->away_msg != NULL)
					ret = xstrncpy(buf, _("away"), len);
				else
					ret = xstrncpy(buf, _("online"), len);
			} else
				ret = xstrncpy(buf, _("not connected"), len);
			break;

		/* Protocol */
		case '?':
			ret = xstrncpy(buf, acct->proto->name, len);
			break;

		/* User mode */
		case 'u':
		case 'U':
			ret = xstrncpy(buf, acct->umode, len);
			break;

		/* Timestamp */
		case 't':
		case 'T':
			ret = fill_format_str(OPT_FORMAT_STATUS_TIMESTAMP, buf, len);
			break;

		/* Activity */
		case 'a':
		case 'A':
			ret = fill_format_str(OPT_FORMAT_STATUS_ACTIVITY, buf, len);
			break;

		/* Typing */
		case 'y':
		case 'Y':
			ret = fill_format_str(OPT_FORMAT_STATUS_TYPING, buf, len, imwindow);
			break;

		/* Held Messages */
		case 'h':
		case 'H':
			ret = fill_format_str(OPT_FORMAT_STATUS_HELD, buf, len, imwindow);
			break;

		/* Idle Time */
		case 'i':
		case 'I':
			ret = fill_format_str(OPT_FORMAT_STATUS_IDLE, buf, len, acct,
					isupper(opt));
			break;

		/* Warn Level */
		case 'w':
		case 'W':
			ret = fill_format_str(OPT_FORMAT_STATUS_WARN, buf, len, acct,
					isupper(opt));
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_status_warn(char opt, char *buf, size_t len, va_list ap) {
	struct pork_acct *acct= va_arg(ap, struct pork_acct *);
	int hide_if_zero = va_arg(ap, int);
	int ret = 0;

	switch (opt) {
		/* Warn level */
		case 'W':
		own:
			if (acct->warn_level == 0 && hide_if_zero)
				return (1);

			ret = snprintf(buf, len, "%02d", acct->warn_level);
			break;

		case 'w': {
			struct imwindow *win = cur_window();

			if (win->type == WIN_TYPE_PRIVMSG) {
				struct buddy *buddy;

				buddy = buddy_find(acct, win->target);
				if (buddy == NULL || (buddy->warn_level == 0 && hide_if_zero))
					return (1);

				ret = snprintf(buf, len, "%02d", buddy->warn_level);
				break;
			}

			goto own;
		}

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_msg_send(char opt, char *buf, size_t len, va_list ap) {
	struct pork_acct *acct = va_arg(ap, struct pork_acct *);
	char *dest = va_arg(ap, char *);
	char *msg = va_arg(ap, char *);
	int ret = 0;

	switch (opt) {
		/* Timestamp */
		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;

		/* Screen name of sender */
		case 'n':
		case 'N':
			ret = xstrncpy(buf, acct->username, len);
			break;

		/* Screen name / alias of the receiver */
		case 'R':
			if (dest != NULL)
				ret = xstrncpy(buf, buddy_name(acct, dest), len);
			break;

		case 'r':
			if (dest != NULL)
				ret = xstrncpy(buf, dest, len);
			break;

		/* Message text */
		case 'M':
			if (msg != NULL) {
				msg = acct->proto->filter_text(msg);
				ret = xstrncpy(buf, msg, len);
				free(msg);
			}
			break;

		case 'm':
			if (msg != NULL)
				ret = xstrncpy(buf, msg, len);
			break;

		case 'H':
			if (acct->userhost != NULL) {
				char *host = acct->proto->filter_text(acct->userhost);
				ret = xstrncpy(buf, host, len);
				free(host);
			}
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_msg_recv(char opt, char *buf, size_t len, va_list ap) {
	struct pork_acct *acct = va_arg(ap, struct pork_acct *);
	char *dest = va_arg(ap, char *);
	char *sender = va_arg(ap, char *);
	char *sender_userhost = va_arg(ap, char *);
	char *msg = va_arg(ap, char *);
	int ret = 0;

	switch (opt) {
		/* Timestamp */
		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;

		/* Screen name of sender */
		case 'n':
			if (sender != NULL)
				ret = xstrncpy(buf, sender, len);
			break;

		/* Screen name / alias of sender */
		case 'N':
			if (sender != NULL)
				ret = xstrncpy(buf, buddy_name(acct, sender), len);
			break;

		/* Screen name / alias of the receiver */
		case 'R':
			if (dest != NULL)
				ret = xstrncpy(buf, buddy_name(acct, dest), len);
			break;

		case 'r':
			if (dest != NULL)
				ret = xstrncpy(buf, dest, len);
			break;

		/* Message text */
		case 'm':
		case 'M':
			if (msg != NULL) {
				msg = acct->proto->filter_text(msg);
				ret = xstrncpy(buf, msg, len);
				free(msg);
			}
			break;

		case 'H':
			if (acct->userhost != NULL) {
				char *host = acct->proto->filter_text(acct->userhost);
				ret = xstrncpy(buf, host, len);
				free(host);
			}
			break;

		case 'h':
			if (sender_userhost != NULL) {
				char *host = acct->proto->filter_text(sender_userhost);
				ret = xstrncpy(buf, host, len);
				free(host);
			}
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_chat_send(char opt, char *buf, size_t len, va_list ap) {
	struct pork_acct *acct = va_arg(ap, struct pork_acct *);
	struct chatroom *chat = va_arg(ap, struct chatroom *);
	char *dest = va_arg(ap, char *);
	char *msg = va_arg(ap, char *);

	int ret = 0;

	switch (opt) {
		/* Timestamp */
		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;

		/* Message source */
		case 'n':
		case 'N':
			ret = xstrncpy(buf, acct->username, len);
			break;

		/* Message destination */
		case 'C':
			if (chat != NULL)
				ret = xstrncpy(buf, chat->title_quoted, len);
			break;

		case 'c':
			if (dest != NULL) {
				dest = acct->proto->filter_text(dest);
				ret = xstrncpy(buf, dest, len);
				free(dest);
			}
			break;

		/* Message text */
		case 'm':
		case 'M':
			if (msg != NULL) {
				msg = acct->proto->filter_text(msg);
				ret = xstrncpy(buf, msg, len);
				free(msg);
			}
			break;

		case 'H':
			if (acct->userhost != NULL) {
				char *host = acct->proto->filter_text(acct->userhost);
				ret = xstrncpy(buf, host, len);
				free(host);
			}
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_chat_recv(char opt, char *buf, size_t len, va_list ap) {
	struct pork_acct *acct = va_arg(ap, struct pork_acct *);
	struct chatroom *chat = va_arg(ap, struct chatroom *);
	char *dest = va_arg(ap, char *);
	char *src = va_arg(ap, char *);
	char *src_uhost = va_arg(ap, char *);
	char *msg = va_arg(ap, char *);

	int ret = 0;

	switch (opt) {
		/* Timestamp */
		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;

		/* Message source */
		case 'N':
			if (src != NULL)
				ret = xstrncpy(buf, buddy_name(acct, src), len);
			break;

		case 'n':
			if (src != NULL)
				ret = xstrncpy(buf, src, len);
			break;

		/* Message destination */
		case 'C':
			if (chat != NULL)
				ret = xstrncpy(buf, chat->title_quoted, len);
			break;

		case 'c':
			if (dest != NULL) {
				dest = acct->proto->filter_text(dest);
				ret = xstrncpy(buf, dest, len);
				free(dest);
			}
			break;

		/* Message text */
		case 'M':
		case 'm':
			if (msg != NULL) {
				msg = acct->proto->filter_text(msg);
				ret = xstrncpy(buf, msg, len);
				free(msg);
			}
			break;

		case 'H':
			if (acct->userhost != NULL) {
				char *host = acct->proto->filter_text(acct->userhost);
				ret = xstrncpy(buf, host, len);
				free(host);
			}
			break;

		case 'h':
			if (src_uhost != NULL) {
				src_uhost = acct->proto->filter_text(src_uhost);
				ret = xstrncpy(buf, src_uhost, len);
				free(src_uhost);
			}
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_chat_info(char opt, char *buf, size_t len, va_list ap) {
	struct pork_acct *acct = va_arg(ap, struct pork_acct *);
	struct chatroom *chat = va_arg(ap, struct chatroom *);
	char *chat_nameq = va_arg(ap, char *);
	char *src = va_arg(ap, char *);
	char *dst = va_arg(ap, char *);
	char *msg = va_arg(ap, char *);
	int ret = 0;

	switch (opt) {
		/* Timestamp */
		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;

		/* Screen name of user who the action originated from */
		case 'n':
		case 'N':
			ret = xstrncpy(buf, buddy_name(acct, src), len);
			break;

		/* Destination, if applicable */
		case 'D':
		case 'd':
			if (dst != NULL) {
				dst = acct->proto->filter_text(dst);
				ret = xstrncpy(buf, dst, len);
				free(dst);
			}
			break;

		/* Chat name (quoted) */
		case 'r':
		case 'R':
			ret = xstrncpy(buf, chat_nameq, len);
			break;

		/* Chat name (full, quoted) */
		case 'U':
		case 'u':
			if (chat != NULL)
				ret = xstrncpy(buf, chat->title_full_quoted, len);
			break;

		/* Source's userhost (if available) */
		case 'H':
			if (acct != NULL && chat != NULL && src != NULL) {
				struct chat_user *chat_user;

				chat_user = chat_find_user(acct, chat, src);
				if (chat_user != NULL && chat_user->host != NULL) {
					char *host = acct->proto->filter_text(chat_user->host);
					ret = xstrncpy(buf, host, len);
					free(host);
				}
			}
			break;

		/* Dest's userhost (if available) */
		case 'h':
			if (acct != NULL && chat != NULL && dst != NULL) {
				struct chat_user *chat_user;

				chat_user = chat_find_user(acct, chat, dst);
				if (chat_user != NULL && chat_user->host != NULL) {
					char *host = acct->proto->filter_text(chat_user->host);
					ret = xstrncpy(buf, host, len);
					free(host);
				}
			}
			break;

		case 'm':
		case 'M':
			if (msg != NULL) {
				msg = acct->proto->filter_text(msg);
				ret = xstrncpy(buf, msg, len);
				free(msg);
			}
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_warning(char opt, char *buf, size_t len, va_list ap) {
	char *warned = va_arg(ap, char *);
	char *warner = va_arg(ap, char *);
	u_int16_t warn_level = va_arg(ap, unsigned int);
	int ret = 0;

	switch (opt) {
		/* Timestamp */
		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;

		case 'n':
		case 'N':
			ret = xstrncpy(buf, warned, len);
			break;

		case 'u':
		case 'U':
			ret = xstrncpy(buf, warner, len);
			break;

		case 'w':
		case 'W':
			ret = snprintf(buf, len, "%u", warn_level);
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_blist_idle(char opt, char *buf, size_t len, va_list ap) {
	struct buddy *buddy = va_arg(ap, struct buddy *);
	int ret = 0;

	switch (opt) {
		case 'I':
			ret = time_to_str(buddy->idle_time, buf, len);
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_blist_warn(char opt, char *buf, size_t len, va_list ap) {
	struct buddy *buddy = va_arg(ap, struct buddy *);
	int ret = 0;

	switch (opt) {
		case 'W':
			ret = snprintf(buf, len, "%u", buddy->warn_level);
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_blist_buddy_label(char opt, char *buf, size_t len, va_list ap)
{
	struct buddy *buddy = va_arg(ap, struct buddy *);
	int ret = 0;

	switch (opt) {
		case 'B':
		case 'b': {
			char *status_text;

			if (buddy->status == STATUS_ACTIVE)
				status_text = opt_get_str(globals.prefs, OPT_TEXT_BUDDY_ACTIVE);
			else if (buddy->status == STATUS_AWAY)
				status_text = opt_get_str(globals.prefs, OPT_TEXT_BUDDY_AWAY);
			else if (buddy->status == STATUS_IDLE)
				status_text = opt_get_str(globals.prefs, OPT_TEXT_BUDDY_IDLE);
			else if (buddy->status == STATUS_WIRELESS)
				status_text = opt_get_str(globals.prefs, OPT_TEXT_BUDDY_WIRELESS);
			else
				status_text = "%p?%x";

			ret = xstrncpy(buf, status_text, len);
			break;
		}

		case 'N':
		case 'n':
			ret = xstrncpy(buf, buddy->name, len);
			break;

		case 'I':
			ret = fill_format_str(OPT_FORMAT_BLIST_IDLE, buf, len, buddy);
			break;

		case 'i':
			if (buddy->idle_time > 0)
				ret = fill_format_str(OPT_FORMAT_BLIST_IDLE, buf, len, buddy);
			break;

		case 'W':
			ret = fill_format_str(OPT_FORMAT_BLIST_WARN, buf, len, buddy);
			break;

		case 'w':
			if (buddy->warn_level > 0)
				ret = fill_format_str(OPT_FORMAT_BLIST_WARN, buf, len, buddy);
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_blist_group_label(char opt, char *buf, size_t len, va_list ap)
{
	struct bgroup *group = va_arg(ap, struct bgroup *);
	dlist_t *node = group->blist_line;
	int ret = 0;

	switch (opt) {
		case 'E': {
			char *expand_str;

			if (node != NULL) {
				struct slist_cell *cell = node->data;

				if (cell != NULL && cell->collapsed) {
					expand_str = opt_get_str(globals.prefs, OPT_TEXT_BLIST_GROUP_COLLAPSED);
				} else {
					expand_str = opt_get_str(globals.prefs, OPT_TEXT_BLIST_GROUP_EXPANDED);
				}
			} else {
				expand_str = opt_get_str(globals.prefs, OPT_TEXT_BLIST_GROUP_COLLAPSED);
			}

			ret = xstrncpy(buf, expand_str, len);
			break;
		}

		case 'N':
		case 'n':
			ret = xstrncpy(buf, group->name, len);
			break;

		case 'T':
		case 't':
			ret = snprintf(buf, len, "%u", group->num_members);
			break;

		case 'O':
		case 'o':
			ret = snprintf(buf, len, "%u", group->num_online);
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static int format_file_transfer(char opt, char *buf, size_t len, va_list ap) {
	struct file_transfer *xfer = va_arg(ap, struct file_transfer *);
	struct pork_acct *acct = xfer->acct;
	int ret = 0;

	switch (opt) {
		/* Timestamp */
		case 'T':
			ret = fill_format_str(OPT_FORMAT_TIMESTAMP, buf, len);
			break;

		/* Starting offset */
		case 'O':
			ret = snprintf(buf, len, "%lld", xfer->start_offset);
			break;

		/* File transfer reference number */
		case 'I':
			ret = snprintf(buf, len, "%u", xfer->refnum);
			break;

		/* Actual file name */
		case 'N':
			ret = xstrncpy(buf, xfer->fname_local, len);
			break;

		/* Original (requested) file name */
		case 'n':
			ret = xstrncpy(buf, xfer->fname_orig, len);
			break;

		/* Local IP address */
		case 'L':
			ret = xstrncpy(buf, xfer->laddr_ip, len);
			break;

		/* Local hostname */
		case 'l':
			ret = xstrncpy(buf, transfer_get_local_hostname(xfer), len);
			break;

		/* Remote IP address */
		case 'F':
			ret = xstrncpy(buf, xfer->faddr_ip, len);
			break;

		/* Remote hostname */
		case 'f':
			ret = xstrncpy(buf, transfer_get_remote_hostname(xfer), len);
			break;

		/* Local port */
		case 'P':
			ret = snprintf(buf, len, "%d", xfer->lport);
			break;

		/* Remote port */
		case 'p':
			ret = snprintf(buf, len, "%d", xfer->fport);
			break;

		/* Average transfer rate */
		case 'R':
			ret = snprintf(buf, len, "%.04f", transfer_avg_speed(xfer));
			break;

		/* File size */
		case 'S':
			ret = snprintf(buf, len, "%lld", xfer->file_len - xfer->start_offset);
			break;

		/* Number of bytes sent */
		case 's':
			ret = snprintf(buf, len, "%lld", xfer->bytes_sent);
			break;

		/* Time elapsed since the start of the transfer in seconds */
		case 't':
			ret = snprintf(buf, len, "%.04f", transfer_time_elapsed(xfer));
			break;

		/* Local user */
		case 'U':
			ret = xstrncpy(buf, acct->username, len);
			break;

		/* Remote user */
		case 'u':
			ret = xstrncpy(buf, xfer->peer_username, len);
			break;

		default:
			return (-1);
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

int (*const global_format_handler[])(char, char *, size_t, va_list) = {
	format_msg_recv,			/* OPT_FORMAT_ACTION_RECV			*/
	format_msg_recv,			/* OPT_FORMAT_ACTION_RECV_STATUS	*/
	format_msg_send,			/* OPT_FORMAT_ACTION_SEND			*/
	format_msg_send,			/* OPT_FORMAT_ACTION_SEND_STATUS	*/
	format_blist_buddy_label,	/* OPT_FORMAT_BLIST					*/
	format_blist_group_label,	/* OPT_FORMAT_BLIST_GROUP			*/
	format_blist_idle,			/* OPT_FORMAT_BLIST_IDLE			*/
	format_blist_warn,			/* OPT_FORMAT_BLIST_WARN			*/
	format_chat_info,			/* OPT_FORMAT_CHAT_CREATE			*/
	format_chat_info,			/* OPT_FORMAT_CHAT_IGNORE			*/
	format_chat_info,			/* OPT_FORMAT_CHAT_INVITE			*/
	format_chat_info,			/* OPT_FORMAT_CHAT_JOIN				*/
	format_chat_info,			/* OPT_FORMAT_CHAT_KICK				*/
	format_chat_info,			/* OPT_FORMAT_CHAT_LEAVE			*/
	format_chat_info,			/* OPT_FORMAT_CHAT_MODE				*/
	format_chat_info,			/* OPT_FORMAT_CHAT_NICK				*/
	format_chat_info,			/* OPT_FORMAT_CHAT_QUIT				*/
	format_chat_recv,			/* OPT_FORMAT_CHAT_RECV				*/
	format_chat_recv,			/* OPT_FORMAT_CHAT_RECV_ACTION		*/
	format_chat_recv,			/* OPT_FORMAT_CHAT_RECV_NOTICE		*/
	format_chat_send,			/* OPT_FORMAT_CHAT_SEND				*/
	format_chat_send,			/* OPT_FORMAT_CHAT_SEND_ACTION		*/
	format_chat_send,			/* OPT_FORMAT_CHAT_SEND_NOTICE		*/
	format_chat_info,			/* OPT_FORMAT_CHAT_TOPIC			*/
	format_chat_info,			/* OPT_FORMAT_CHAT_UNIGNORE			*/
	format_file_transfer,		/* OPT_FORMAT_FILE_CANCEL_LOCAL		*/
	format_file_transfer,		/* OPT_FORMAT_FILE_CANCEL_REMOTE	*/
	format_file_transfer,		/* OPT_FORMAT_FILE_LOST				*/
	format_file_transfer,		/* OPT_FORMAT_FILE_RECV_ACCEPT		*/
	format_file_transfer,		/* OPT_FORMAT_FILE_RECV_ASK			*/
	format_file_transfer,		/* OPT_FORMAT_FILE_RECV_COMPLETE	*/
	format_file_transfer,		/* OPT_FORMAT_FILE_RECV_RESUME		*/
	format_file_transfer,		/* OPT_FORMAT_FILE_SEND_ACCEPT		*/
	format_file_transfer,		/* OPT_FORMAT_FILE_SEND_ASK			*/
	format_file_transfer,		/* OPT_FORMAT_FILE_SEND_COMPLETE	*/
	format_file_transfer,		/* OPT_FORMAT_FILE_SEND_RESUME		*/
	format_msg_recv,			/* OPT_FORMAT_MSG_RECV				*/
	format_msg_recv,			/* OPT_FORMAT_MSG_RECV_AUTO			*/
	format_msg_recv,			/* OPT_FORMAT_MSG_RECV_STATUS		*/
	format_msg_send,			/* OPT_FORMAT_MSG_SEND				*/
	format_msg_send,			/* OPT_FORMAT_MSG_SEND_AUTO			*/
	format_msg_send,			/* OPT_FORMAT_MSG_SEND_STATUS		*/
	format_msg_recv,			/* OPT_FORMAT_NOTICE_RECV			*/
	format_msg_recv,			/* OPT_FORMAT_NOTICE_RECV_STATUS	*/
	format_msg_send,			/* OPT_FORMAT_NOTICE_SEND			*/
	format_msg_send,			/* OPT_FORMAT_NOTICE_SEND_STATUS	*/
	format_status,				/* OPT_FORMAT_STATUS				*/
	format_status_activity,		/* OPT_FORMAT_STATUS_ACTIVITY		*/
	format_status,				/* OPT_FORMAT_STATUS_CHAT			*/
	format_status_held,			/* OPT_FORMAT_STATUS_HELD			*/
	format_status_idle,			/* OPT_FORMAT_STATUS_IDLE			*/
	format_status_timestamp,	/* OPT_FORMAT_STATUS_TIMESTAMP		*/
	format_status_typing,		/* OPT_FORMAT_STATUS_TYPING			*/
	format_status_warn,			/* OPT_FORMAT_STATUS_WARN			*/
	format_status_timestamp,	/* OPT_FORMAT_TIMESTAMP				*/
	format_warning,				/* OPT_FORMAT_WARN_RECV				*/
	format_warning,				/* OPT_FORMAT_WARN_RECV_ANON		*/
	format_warning,				/* OPT_FORMAT_WARN_SEND				*/
	format_warning,				/* OPT_FORMAT_WARN_SEND_ANON		*/
};
