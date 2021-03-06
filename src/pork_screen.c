/*
** pork_screen.c - screen management.
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
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
#include <sys/time.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_misc.h>
#include <pork_imsg.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_buddy.h>
#include <pork_slist.h>
#include <pork_buddy_list.h>
#include <pork_proto.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_acct_set.h>
#include <pork_imwindow_set.h>
#include <pork_chat.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_io.h>
#include <pork_status.h>
#include <pork_ssl.h>

/*
** Find the window having the specified refnum, and return
** a pointer to the node that holds it.
**
** This depends on the list being sorted.
*/

static dlist_t *screen_find_refnum(u_int32_t refnum) {
	dlist_t *cur = globals.window_list;

	do {
		struct imwindow *imwindow = cur->data;

		if (imwindow->refnum == refnum)
			return (cur);

		if (imwindow->refnum > refnum)
			break;

		cur = cur->next;
	} while (cur != globals.window_list);

	return (NULL);
}

/*
** Yes, this is pretty slow and stupid, but it'd be pretty
** unusual (i think) to have even 10 windows open at any one time.
**
** It's also only called when creating a new window.
*/

static inline u_int32_t screen_get_new_refnum(void) {
	u_int32_t i;

	for (i = 1 ; i < 0xffffffff ; i++) {
		if (screen_find_refnum(i) == NULL)
			return (i);
	}

	return (0);
}

static void screen_window_list_add(dlist_t *new_node) {
	struct imwindow *imwindow = new_node->data;

	/*
	** The window list is a sorted circular doubly linked list.
	** The window_list pointer points to the window having the lowest
	** refnum.
	*/

	if (globals.window_list == NULL) {
		new_node->prev = new_node;
		new_node->next = new_node;
		globals.window_list = new_node;
	} else {
		dlist_t *cur = globals.window_list;

		do {
			struct imwindow *imw = cur->data;

			if (imwindow->refnum < imw->refnum) {
				if (cur == globals.window_list)
					globals.window_list = new_node;
				break;
			}

			cur = cur->next;
		} while (cur != globals.window_list);

		new_node->next = cur;
		new_node->prev = cur->prev;

		new_node->next->prev = new_node;
		new_node->prev->next = new_node;
	}
}

static void screen_window_list_remove(dlist_t *node) {
	dlist_t *save = node;

	if (node == globals.window_list)
		globals.window_list = node->next;

	if (node == globals.window_list)
		globals.window_list = NULL;

	save->prev->next = node->next;
	save->next->prev = node->prev;
}

/*
** Create the program's status window. This window will have the buddy
** list visible in it by default.
*/

int screen_init(u_int32_t rows, u_int32_t cols) {
	struct imwindow *win;
	struct pork_acct *acct;

	memset(&globals, 0, sizeof(globals));

	globals.rows = rows;
	globals.cols = cols;

	init_global_prefs(&globals);
	bind_init(&globals.binds);

	if (status_init() == -1)
		return (-1);

	acct = pork_acct_init(opt_get_str(globals.prefs, OPT_TEXT_NO_NAME),
			PROTO_NULL);
	if (acct == NULL)
		return (-1);
	acct->refnum = -1;

	pork_acct_add(acct);
	globals.null_acct = acct;

	pork_io_add(STDIN_FILENO, IO_COND_READ, &globals, &globals,
		keyboard_input);

	rows = max(1, (int) rows - STATUS_ROWS);

	win = imwindow_new(rows, cols, 1, WIN_TYPE_STATUS, acct, _("status"));
	if (win == NULL)
		return (-1);
	input_init(&globals.input, cols);

	screen_add_window(win);
	opt_set(win->prefs, WIN_OPT_SHOW_BLIST, "1");
	globals.status_win = win;
	return (0);
}

void screen_destroy(void) {
	dlist_t *cur = globals.window_list;

	do {
		dlist_t *next = cur->next;

		imwindow_destroy(cur->data);
		free(cur);

		cur = next;
	} while (cur != globals.window_list);

	opt_destroy(globals.prefs);
	input_destroy(&globals.input);
	bind_destroy(&globals.binds);
	hash_destroy(&globals.alias_hash);
	delwin(globals.status_bar);
	wclear(curscr);
	if (globals.ssl_ctx != NULL)
		SSL_CTX_free(globals.ssl_ctx);
}

void screen_add_window(struct imwindow *imwindow) {
	dlist_t *new_node = xmalloc(sizeof(*new_node));

	new_node->data = imwindow;
	screen_window_list_add(new_node);

	/*
	** If this is the first window, make it current.
	*/

	if (globals.cur_window == NULL)
		screen_window_swap(new_node);
}

/*
** Change the refnum on the currently visible window.
*/

int screen_renumber(struct imwindow *imwindow, u_int32_t refnum) {
	dlist_t *node;
	u_int32_t old_refnum = imwindow->refnum;

	node = screen_find_refnum(old_refnum);
	if (node == NULL)
		return (-1);

	screen_window_list_remove(node);
	imwindow->refnum = refnum;

	/*
	** If there's more than one window, check to
	** make sure that no other window's refnum
	** is equal to the refnum we just set for 'imwindow'.
	** If it is, give it 'imwindow's' old refnum.
	*/

	if (node != node->next || node->next != node->prev) {
		dlist_t *temp = screen_find_refnum(refnum);
		if (temp != NULL) {
			struct imwindow *imw;

			screen_window_list_remove(temp);
			imw = temp->data;
			imw->refnum = old_refnum;
			screen_window_list_add(temp);
			screen_win_msg(imw, 0, 1, 1, MSG_TYPE_CMD_OUTPUT,
				_("This is now window %%W%u"), old_refnum);
		}
	}

	screen_window_list_add(node);
	screen_cmd_output(_("This is now window %%W%u"), imwindow->refnum);
	return (0);
}

void screen_resize(u_int32_t rows, u_int32_t cols) {
	dlist_t *cur;
	int ret;

	resize_terminal(rows, cols);

	globals.rows = rows;
	globals.cols = cols;

	for (cur = globals.acct_list ; cur != NULL ; cur = cur->next) {
		struct pork_acct *acct = cur->data;

		if (acct->blist != NULL) {
			blist_resize(acct->blist, max(1, (int) rows - STATUS_ROWS),
				acct->blist->slist.cols, cols);
		}
	}

	cur = globals.window_list;
	do {
		struct imwindow *imwindow = cur->data;
		u_int32_t im_cols = cols;

		if (imwindow->blist_visible) {
			u_int32_t blist_cols = imwindow->owner->blist->slist.cols;

			if (blist_cols >= cols) {
				imwindow->blist_visible = 0;
				imwindow->active_binds = &globals.binds.main;
			} else
				im_cols -= blist_cols;
		}

		imwindow_resize(imwindow,
			max(1, (int) rows - STATUS_ROWS), im_cols);
		input_resize(imwindow->input, cols);

		cur = cur->next;
	} while (globals.window_list != cur);

	ret = mvwin(globals.status_bar, max(0, (int) rows - STATUS_ROWS), 0);
	if (ret == -1) {
		delwin(globals.status_bar);
		status_init();
	}
}

int screen_blist_width(struct blist *blist, u_int32_t new_width) {
	dlist_t *cur;

	if (new_width < 3 || new_width >= globals.cols)
		return (-1);

	if (blist == NULL)
		return (-1);

	cur = globals.window_list;
	do {
		struct imwindow *imwindow = cur->data;

		if (imwindow->owner != NULL && imwindow->owner->blist == blist &&
			imwindow->blist_visible)
		{
			imwindow_resize(imwindow, imwindow->swindow.rows,
				imwindow->swindow.cols + blist->slist.cols - new_width);
		}
		cur = cur->next;
	} while (cur != globals.window_list);

	blist_resize(blist, blist->slist.rows, new_width, globals.cols);
	return (0);
}

void screen_window_swap(dlist_t *new_cur) {
	struct imwindow *imwindow = NULL;
	u_int32_t last_own_input = 0;
	u_int32_t cur_own_input;
	struct pork_acct *acct;

	if (globals.cur_window != NULL) {
		imwindow = cur_window();

		last_own_input = opt_get_bool(imwindow->prefs, WIN_OPT_PRIVATE_INPUT);
		imwindow->swindow.activity = 0;
		imwindow->swindow.visible = 0;
	}

	globals.cur_window = new_cur;

	imwindow = cur_window();
	imwindow->swindow.visible = 1;
	imwindow->swindow.activity = 0;
	cur_own_input = opt_get_bool(imwindow->prefs, WIN_OPT_PRIVATE_INPUT);

	/*
	** If the current window and the one that we're going to switch
	** to don't share an input buffer, history, etc, the input
	** line must be refreshed. Force a redraw.
	*/

	if (cur_own_input || last_own_input) {
		imwindow->input->dirty = 1;
		screen_draw_input();
	}

	status_draw(imwindow->owner);

	acct = imwindow->owner;

	/*
	** To force ncurses to redraw it on the physical screen.
	*/
	redrawwin(imwindow->swindow.win);
	imwindow->swindow.dirty = 1;
	imwindow_blist_draw(imwindow);
}

inline int screen_goto_window(u_int32_t refnum) {
	dlist_t *cur = screen_find_refnum(refnum);

	if (cur == NULL)
		return (-1);

	screen_window_swap(cur);
	return (0);
}

void screen_refresh(void) {
	struct imwindow *imwindow = cur_window();

	wclear(curscr);
	wnoutrefresh(curscr);
	wclear(imwindow->swindow.win);
	wnoutrefresh(imwindow->swindow.win);
	swindow_redraw(&imwindow->swindow);
	imwindow_blist_draw(imwindow);
	status_draw(imwindow->owner);
	imwindow_refresh(imwindow);
	imwindow->input->dirty = 1;
	screen_draw_input();
	screen_doupdate();
}

struct imwindow *screen_new_window(	struct pork_acct *acct,
									char *target,
									char *name)
{
	u_int32_t refnum = screen_get_new_refnum();
	struct imwindow *imwindow;
	u_int32_t rows;

	rows = max(1, (int) globals.rows - STATUS_ROWS);

	imwindow = imwindow_new(rows, globals.cols,
		refnum, WIN_TYPE_PRIVMSG, acct, target);
	if (imwindow == NULL)
		return (NULL);

	if (name != NULL)
		imwindow_rename(imwindow, name);

	screen_add_window(imwindow);
	status_draw(imwindow->owner);

	return (imwindow);
}

struct imwindow *screen_new_chat_window(struct pork_acct *acct, char *name) {
	u_int32_t refnum = screen_get_new_refnum();
	struct imwindow *imwindow;
	u_int32_t rows;

	rows = max(1, (int) globals.rows - STATUS_ROWS);
	imwindow = imwindow_new(rows, globals.cols,
		refnum, WIN_TYPE_CHAT, acct, name);
	if (imwindow == NULL)
		return (NULL);

	imwindow->data = NULL;

	screen_add_window(imwindow);
	status_draw(imwindow->owner);

	return (imwindow);
}

int screen_get_query_window(struct pork_acct *acct,
							char *name,
							struct imwindow **winr)
{
	struct imwindow *win;
	int new = 0;

	win = imwindow_find(acct, name);
	if (win == NULL || win->type != WIN_TYPE_PRIVMSG) {
		if (opt_get_bool(acct->prefs, ACCT_OPT_DUMP_MSGS_TO_STATUS))
			win = globals.status_win;
		else {
			win = screen_new_window(acct, name, buddy_name(acct, name));
			new++;
		}
	}

	*winr = win;
	return (new);
}

/*
** When we really want a query window.
*/

int screen_make_query_window(struct pork_acct *acct,
							char *name,
							struct imwindow **winr)
{
	struct imwindow *win;
	int new = 0;

	win = imwindow_find(acct, name);
	if (win == NULL || win->type != WIN_TYPE_PRIVMSG) {
		win = screen_new_window(acct, name, buddy_name(acct, name));
		new++;
	}

	*winr = win;
	return (new);
}

void screen_cycle_fwd(void) {
	dlist_t *cur = globals.cur_window;
	struct imwindow *win;

	do {
		cur = cur->next;
		win = cur->data;
	} while (win->skip && cur != globals.cur_window);

	screen_window_swap(cur);
}

void screen_cycle_fwd_active(void) {
	dlist_t *cur = globals.cur_window;
	struct imwindow *win;

	do {
		cur = cur->next;
		win = cur->data;
	} while (win->skip && !win->swindow.activity && cur != globals.cur_window);

	screen_window_swap(cur);
}

void screen_cycle_bak(void) {
	dlist_t *cur = globals.cur_window;
	struct imwindow *win;

	do {
		cur = cur->prev;
		win = cur->data;
	} while (win->skip && cur != globals.cur_window);

	screen_window_swap(cur);
}

void screen_cycle_bak_active(void) {
	dlist_t *cur = globals.cur_window;
	struct imwindow *win;

	do {
		cur = cur->prev;
		win = cur->data;
	} while (win->skip && !win->swindow.activity && cur != globals.cur_window);

	screen_window_swap(cur);
}

void screen_bind_all_unbound(struct pork_acct *acct) {
	dlist_t *node;

	node = globals.window_list;

	do {
		struct imwindow *win = node->data;

		if (win->owner == globals.null_acct) {
			imwindow_bind_acct(win, acct->refnum);

			if (acct->blist != NULL && !win->blist_visible) {
				if (opt_get_bool(win->prefs, WIN_OPT_SHOW_BLIST))
					opt_set(win->prefs, WIN_OPT_SHOW_BLIST, "1");
				}
			}

		node = node->next;
	} while (node != globals.window_list);
}

int screen_close_window(struct imwindow *imwindow) {
	dlist_t *node = screen_find_refnum(imwindow->refnum);

	if (node == NULL)
		return (-1);

	/*
	** The status window can't be closed.
	*/

	if (imwindow->type == WIN_TYPE_STATUS)
		return (-1);

	if (imwindow->type == WIN_TYPE_CHAT && imwindow->data != NULL) {
		struct chatroom *chat = imwindow->data;

		chat_leave(imwindow->owner, chat->title, 0);
	}

	if (imwindow->swindow.visible)
		screen_cycle_fwd();

	screen_window_list_remove(node);
	imwindow_destroy(imwindow);
	free(node);

	return (0);
}
