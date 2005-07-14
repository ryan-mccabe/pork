/*
** pork_imwindow.c - interface for manipulating conversation/chat/info windows.
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_imsg.h>
#include <pork_set.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_imwindow_set.h>
#include <pork_slist.h>
#include <pork_buddy_list.h>
#include <pork_html.h>
#include <pork_proto.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_cstr.h>
#include <pork_color.h>
#include <pork_misc.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_format.h>
#include <pork_chat.h>

extern struct screen screen;

struct imwindow *imwindow_new(	u_int32_t rows,
								u_int32_t cols,
								u_int32_t refnum,
								u_int32_t type,
								struct pork_acct *owner,
								char *target)
{
	WINDOW *swin;
	struct imwindow *win;
	struct pork_input *input;
	char nname[NUSER_LEN];

	swin = newwin(rows, cols, 0, 0);
	if (swin == NULL) {
		debug("Unable to create %ux%u window", rows, cols);
		return (NULL);
	}

	owner->proto->normalize(nname, target, sizeof(nname));

	win = xcalloc(1, sizeof(*win));
	win->refnum = refnum;
	win->type = type;
	win->target = xstrdup(nname);
	win->name = color_quote_codes(target);
	win->active_binds = &screen.binds.main;
	win->owner = owner;
	win->owner->ref_count++;

	if (imwindow_init_prefs(win) == -1)
		imwindow_destroy(win);

	swindow_init(&win->swindow, swin, rows, cols, win->prefs);

	/*
	** Use a separate input handler.
	*/

	if (opt_get_bool(win->prefs, WIN_OPT_PRIVATE_INPUT)) {
		input = xmalloc(sizeof(*input));
		input_init(input, cols);
	} else
		input = &screen.input;

	win->input = input;

	if (opt_get_bool(win->prefs, WIN_OPT_SHOW_BLIST)) {
		if (owner->blist != NULL)
			imwindow_blist_show(win);
	}

	return (win);
}

inline void imwindow_rename(struct imwindow *win, char *new_name) {
	free(win->name);
	win->name = color_quote_codes(new_name);
}

inline void imwindow_resize(struct imwindow *win,
							u_int32_t rows,
							u_int32_t cols)
{
	swindow_resize(&win->swindow, rows, cols);
	swindow_scroll_to_start(&win->swindow);
	swindow_scroll_to_end(&win->swindow);
}

int imwindow_set_priv_input(struct imwindow *win, int val) {
	int old_val;

	old_val = !(win->input == &screen.input);
	if (old_val == val)
		return (-1);

	/*
	** Give this imwindow its own input buffer and history.
	*/
	if (val == 1) {
		struct pork_input *input = xmalloc(sizeof(*input));

		input_init(input, win->swindow.cols);
		win->input = input;
	} else {
		/*
		** Destroy this imwindow's private input, and set its input
		** to the global input.
		*/

		input_destroy(win->input);
		free(win->input);
		win->input = &screen.input;
	}

	return (0);
}

inline int imwindow_blist_refresh(struct imwindow *win) {
	struct pork_acct *owner = win->owner;

	if (!win->blist_visible)
		return (0);

	if (!owner->blist->slist.dirty)
		return (0);

	blist_draw_border(owner->blist, win->input_focus);
	blist_refresh(owner->blist);
	return (1);
}

inline int imwindow_refresh(struct imwindow *win) {
	int was_dirty_win;
	int was_dirty_blist;

	was_dirty_win = swindow_refresh(&win->swindow);
	was_dirty_blist = imwindow_blist_refresh(win);

	return (was_dirty_win || was_dirty_blist);
}

void imwindow_blist_show(struct imwindow *win) {
	u_int32_t new_width;

	if (win->owner->blist == NULL)
		return;

	if (win->blist_visible)
		return;

	win->blist_visible = 1;
	win->owner->blist->slist.dirty = 1;
	new_width = win->swindow.cols - win->owner->blist->slist.cols;
	imwindow_resize(win, win->swindow.rows, new_width);
}

void imwindow_buffer_find(struct imwindow *win, char *str, u_int32_t opt) {
	screen_win_msg(cur_window(), 1, 1, 0, MSG_TYPE_LASTLOG, "Matching lines:");
	swindow_print_matching(&win->swindow, str, opt);
	screen_win_msg(cur_window(), 1, 1, 0, MSG_TYPE_LASTLOG, "End of matches");
}

void imwindow_blist_hide(struct imwindow *win) {
	u_int32_t new_width;

	if (!win->blist_visible)
		return;

	win->blist_visible = 0;
	if (win->owner->blist != NULL) {
		win->owner->blist->slist.dirty = 1;
		new_width = win->swindow.cols + win->owner->blist->slist.cols;
	} else
		new_width = screen.cols;

	win->input_focus = BINDS_MAIN;
	win->active_binds = &screen.binds.main;

	imwindow_resize(win, win->swindow.rows, new_width);
}

void imwindow_blist_toggle(struct imwindow *win) {
	if (!win->blist_visible)
		imwindow_blist_show(win);
	else
		imwindow_blist_hide(win);
}

void imwindow_blist_draw(struct imwindow *win) {
	struct blist *blist = win->owner->blist;

	if (blist == NULL || !win->blist_visible)
		return;

	blist_draw(win->owner->blist);
}

void imwindow_destroy(struct imwindow *win) {
	swindow_destroy(&win->swindow);

	if (opt_get_bool(win->prefs, WIN_OPT_PRIVATE_INPUT)) {
		input_destroy(win->input);
		free(win->input);
	}

	if (win->owner != NULL)
		win->owner->ref_count--;

	opt_destroy(win->prefs);
	free(win->prefs->val);
	free(win->prefs);
	free(win->name);
	free(win->target);
	free(win);
}

void imwindow_switch_focus(struct imwindow *win) {
	if (!win->blist_visible)
		return;

	if (win->input_focus == BINDS_MAIN) {
		win->input_focus = BINDS_BUDDY;
		win->active_binds = &screen.binds.blist;
	} else {
		win->input_focus = BINDS_MAIN;
		win->active_binds = &screen.binds.main;
	}
}

struct imwindow *imwindow_find(struct pork_acct *owner, const char *target) {
	dlist_t *list_start = screen.window_list;
	dlist_t *cur = list_start;
	char nname[NUSER_LEN];

	owner->proto->normalize(nname, target, sizeof(nname));

	do {
		struct imwindow *win = cur->data;

		if (win->owner == owner &&
			win->type == WIN_TYPE_PRIVMSG &&
			!strcasecmp(win->target, nname))
		{
			return (win);
		}

		cur = cur->next;
	} while (cur != list_start);

	return (NULL);
}

struct imwindow *imwindow_find_chat_target(	struct pork_acct *owner,
											const char *target)
{
	dlist_t *list_start = screen.window_list;
	dlist_t *cur = list_start;
	char nname[NUSER_LEN];

	owner->proto->normalize(nname, target, sizeof(nname));

	do {
		struct imwindow *win = cur->data;

		if (win->owner == owner &&
			win->type == WIN_TYPE_CHAT &&
			!strcasecmp(win->target, nname))
		{
			return (win);
		}

		cur = cur->next;
	} while (cur != list_start);

	return (NULL);
}

struct imwindow *imwindow_find_name(struct pork_acct *owner, const char *name) {
	dlist_t *list_start = screen.window_list;
	dlist_t *cur = list_start;

	do {
		struct imwindow *win = cur->data;

		if (win->owner == owner && !strcasecmp(win->name, name))
			return (win);

		cur = cur->next;
	} while (cur != list_start);

	return (NULL);
}

struct imwindow *imwindow_find_refnum(u_int32_t refnum) {
	dlist_t *cur = screen.window_list;

	do {
		struct imwindow *win = cur->data;

		if (win->refnum == refnum)
			return (win);

		cur = cur->next;
	} while (cur != screen.window_list);

	return (NULL);
}

inline void imwindow_send_msg(struct imwindow *win) {
	swindow_input(&win->swindow);
}

inline void imwindow_recv_msg(struct imwindow *win) {
	if (opt_get_bool(win->prefs, WIN_OPT_BEEP_ON_OUTPUT))
		beep();
}

/*
** Bind the account whose reference number is "refnum" to the window
** "imwindow".
*/

int imwindow_bind_acct(struct imwindow *win, u_int32_t refnum) {
	struct pork_acct *owner;
	struct pork_acct *old_acct = win->owner;

	if (win->type == WIN_TYPE_CHAT &&
		win->owner != screen.null_acct)
	{
		return (-1);
	}

	owner = pork_acct_get_data(refnum);
	if (owner == NULL)
		return (-1);

	if (old_acct != owner) {
		win->owner->ref_count--;
		win->owner = owner;
		win->owner->ref_count++;

		if (win->blist_visible) {
			if (win->owner->blist == NULL)
				imwindow_blist_hide(win);
			else {
				imwindow_blist_draw(win);
				imwindow_blist_refresh(win);
			}
		}
	}

	return (0);
}

inline int imwindow_dump_buffer(struct imwindow *win, char *file) {
	return (swindow_dump_buffer(&win->swindow, file));
}

/*
** Binds the next account in this window.
*/

int imwindow_bind_next_acct(struct imwindow *win) {
	u_int32_t next_refnum;

	if (pork_acct_next_refnum(win->owner->refnum, &next_refnum) == -1)
		return (-1);

	return (imwindow_bind_acct(win, next_refnum));
}

inline void imwindow_scroll_up(struct imwindow *win) {
	swindow_scroll_by(&win->swindow, -1);
}

inline void imwindow_scroll_down(struct imwindow *win) {
	swindow_scroll_by(&win->swindow, 1);
}

inline void imwindow_scroll_by(struct imwindow *win, int lines) {
	swindow_scroll_by(&win->swindow, lines);
}

inline void imwindow_scroll_page_up(struct imwindow *win) {
	swindow_scroll_by(&win->swindow, -win->swindow.rows);
}

inline void imwindow_scroll_page_down(struct imwindow *win) {
	swindow_scroll_by(&win->swindow, win->swindow.rows);
}

inline void imwindow_scroll_start(struct imwindow *win) {
	swindow_scroll_to_start(&win->swindow);
}

inline void imwindow_scroll_end(struct imwindow *win) {
	swindow_scroll_to_end(&win->swindow);
}

inline void imwindow_clear(struct imwindow *win) {
	swindow_clear(&win->swindow);
}

inline void imwindow_erase(struct imwindow *win) {
	swindow_erase(&win->swindow);
}

inline int imwindow_ignore(struct imwindow *win) {
	int ret = win->ignore_activity;

	win->ignore_activity = 1;
	return (ret);
}

inline int imwindow_unignore(struct imwindow *win) {
	int ret = win->ignore_activity;

	win->ignore_activity = 0;
	return (ret);
}

inline int imwindow_skip(struct imwindow *win) {
	int ret = win->skip;

	win->skip = 1;
	return (ret);
}

inline int imwindow_unskip(struct imwindow *win) {
	int ret = win->skip;

	win->skip = 0;
	return (ret);
}

inline int imwindow_add(struct imwindow *win,
						struct imsg *imsg,
						u_int32_t type)
{
	return (swindow_add(&win->swindow, imsg, type));
}
