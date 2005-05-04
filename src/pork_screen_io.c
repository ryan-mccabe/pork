/*
** pork_screen_io.c - screen management.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
**
** The functions in this file are mostly wrappers around
** functions that do the actual work. The difference is
** these functions do any necessary updating of the contents
** of the screen (i.e. refreshing, redrawing, etc).
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_imsg.h>
#include <pork_imwindow.h>
#include <pork_proto.h>
#include <pork_acct.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_format.h>
#include <pork_chat.h>
#include <pork_status.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

inline void screen_doupdate(void) {
	int cur_old = curs_set(0);

	wmove(screen.status_bar, STATUS_ROWS - 1,
		input_get_cursor_pos(cur_window()->input));
	wnoutrefresh(screen.status_bar);

	doupdate();
	curs_set(cur_old);
}

int screen_draw_input(void) {
	struct input *input = cur_window()->input;

	if (input->dirty) {
		char *input_line;
		int len;

		len = input->width;
		input_line = input_partial(input);

		if (input_line == NULL)
			return (0);

		wmove(screen.status_bar, STATUS_ROWS - 1, 0);
		wclrtoeol(screen.status_bar);

		if (input_line == input->input_buf && input->prompt != NULL) {
			wputnstr(screen.status_bar, input->prompt,
				min(input->width, input->prompt_len));
			len -= input->prompt_len;
		}

		if (len < 0)
			return (1);

		wputncstr(screen.status_bar, input_line, len);

		/*
		** We don't need a wnoutfresh here for this window. Since it's
		** dirty (implying we return 1), screen_doupdate() will be called,
		** which redraws the cursor and calls wnoutrefresh on the status
		** window, which happens to be the window the input is drawn in.
		*/

		input->dirty = 0;
		return (1);
	}

	return (0);
}

inline int screen_set_quiet(int status) {
	int ret = screen.quiet;

	screen.quiet = status;
	return (ret);
}

int screen_prompt_user(char *prompt, char *buf, size_t len) {
	int ret;

	buf[0] = '\0';

	wmove(screen.status_bar, 1, 0);
	wclrtoeol(screen.status_bar);

	if (prompt != NULL)
		waddstr(screen.status_bar, prompt);

	wrefresh(screen.status_bar);

	ret = wgetnstr(screen.status_bar, buf, len);
	return (ret);
}

static void __screen_win_msg(	struct imwindow *win,
								u_int32_t opt,
								u_int32_t msgtype,
								char *fmt,
								va_list ap)
{
	char *buf = NULL;
	chtype *ch;
	size_t chlen = 0;
	int (*cstr_conv)(chtype *, size_t, ...) = plaintext_to_cstr;
	char tstxt[128];
	char *banner_txt = "\0";
	char *p;
	int ret;

	if (!(opt & MSG_OPT_COLOR))
		cstr_conv = plaintext_to_cstr_nocolor;

	if (opt & MSG_OPT_TIMESTAMP) {
		ret = fill_format_str(OPT_FORMAT_STATUS_TIMESTAMP, tstxt, sizeof(tstxt));
		if (ret < 1)
			tstxt[0] = '\0';
		else
			chlen += ret;
	} else
		tstxt[0] = '\0';

	if (opt & MSG_OPT_BANNER) {
		banner_txt = opt_get_str(OPT_BANNER);
		if (banner_txt == NULL)
			banner_txt = "\0";
		else
			chlen += strlen(banner_txt);
	}

	ret = vasprintf(&buf, fmt, ap);
	if (ret < 0) {
		debug("unable to allocate string for %s", fmt);
		return;
	}

	p = strchr(buf, '\n');
	if (p != NULL) {
		*p++ = '\0';
		chlen += ret - (p - buf);
	} else
		chlen += ret + 1;

	/*
	** This is a horrible, horrible hack to handle
	** embedded tabs.
	*/
	chlen += 128;
	ch = xmalloc(sizeof(chtype) * chlen);

	chlen = cstr_conv(ch, chlen, tstxt, banner_txt, buf, NULL);
	ch = xrealloc(ch, sizeof(chtype) * (chlen + 1));
	imwindow_add(win, imsg_new(&win->swindow, ch, chlen), msgtype);

	while (p != NULL) {
		char *next;
		size_t len;

		next = strchr(p, '\n');
		if (next != NULL) {
			*next++ = '\0';
			len = next - p + 1;
		} else
			len = strlen(p);

		len += 128;
		ch = xmalloc(sizeof(chtype) * len);

		/* XXX - this should be configurable */
		len = cstr_conv(ch, len, " ", p, NULL);

		ch = xrealloc(ch, sizeof(chtype) * (len + 1));
		imwindow_add(win, imsg_new(&win->swindow, ch, len), msgtype);

		p = next;
	}

	free(buf);
}

void screen_print_str(struct imwindow *win, char *buf, size_t chlen, int type) {
	char *p;
	chtype *ch;

	p = strchr(buf, '\n');
	if (p != NULL) {
		chlen = p - buf - 1;
		*p++ = '\0';
	}

	chlen += 128;

	ch = xmalloc(sizeof(chtype) * (chlen + 1));
	chlen = plaintext_to_cstr(ch, chlen + 1, buf, NULL);
	ch = xrealloc(ch, sizeof(chtype) * (chlen + 1));
	imwindow_add(win, imsg_new(&win->swindow, ch, chlen), type);

	while (p != NULL) {
		chtype *ch;
		char *next;
		size_t len;

		next = strchr(p, '\n');
		if (next != NULL) {
			*next++ = '\0';
			len = next - p + 1;
		} else
			len = strlen(p) + 1;

		len += 128;
		ch = xmalloc(sizeof(chtype) * len);
		len = plaintext_to_cstr(ch, len, " ", p, NULL);
		ch = xrealloc(ch, sizeof(chtype) * (len + 1));
		imwindow_add(win, imsg_new(&win->swindow, ch, len), type);

		p = next;
	}
}

inline void screen_win_msg(	struct imwindow *win,
							int ts,
							int banner,
							int color,
							int type,
							char *fmt,
							...)
{
	va_list ap;

	va_start(ap, fmt);
	__screen_win_msg(win,
		(ts * MSG_OPT_TIMESTAMP) | (banner * MSG_OPT_BANNER) | (color * MSG_OPT_COLOR),
		1, fmt, ap);
	va_end(ap);
}

inline void screen_err_msg(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	__screen_win_msg(cur_window(),
		MSG_OPT_TIMESTAMP | MSG_OPT_BANNER | MSG_OPT_COLOR,
		MSG_TYPE_ERROR, fmt, ap);
	va_end(ap);
}

inline void screen_cmd_output(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	if (!screen.quiet) {
		__screen_win_msg(cur_window(), MSG_OPT_BANNER | MSG_OPT_COLOR,
			MSG_TYPE_CMD_OUTPUT, fmt, ap);
	}
	va_end(ap);
}

inline void screen_nocolor_msg(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	if (!screen.quiet)
		__screen_win_msg(cur_window(), MSG_OPT_BANNER, 1, fmt, ap);
	va_end(ap);
}
