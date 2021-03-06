/*
** pork_screen.h - screen management.
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_SCREEN_H
#define __PORK_SCREEN_H

extern struct screen globals;

enum {
	FOCUS_MSG,
	FOCUS_BUDDY
};

struct blist;
struct imwindow;
struct pork_acct;

struct screen {
	u_int32_t rows;
	u_int32_t cols;
	dlist_t *cur_window;
	dlist_t *window_list;
	dlist_t *timer_list;
	dlist_t *transfer_list;
	struct imwindow *status_win;
	dlist_t *acct_list;
	WINDOW *status_bar;
	u_int32_t quiet:1;
	struct pork_acct *null_acct;
	struct pork_input input;
	struct binds binds;
	hash_t alias_hash;
	void *ssl_ctx;
	struct pref_val *prefs;
};

#define cur_window() ((struct imwindow *) (globals.cur_window->data))

int screen_init(u_int32_t rows, u_int32_t cols);
void screen_destroy(void);

int screen_renumber(struct imwindow *imwindow, u_int32_t refnum);
void screen_add_window(struct imwindow *imwindow);
void screen_resize(u_int32_t rows, u_int32_t cols);
int screen_blist_width(struct blist *blist, u_int32_t new_width);
void screen_window_swap(dlist_t *new_cur);
int screen_goto_window(u_int32_t refnum);
void screen_refresh(void);
void screen_bind_all_unbound(struct pork_acct *acct);
struct imwindow *screen_new_window(struct pork_acct *o, char *dest, char *name);
struct imwindow *screen_new_chat_window(struct pork_acct *acct, char *name);
int screen_close_window(struct imwindow *imwindow);
void screen_cycle_fwd(void);
void screen_cycle_fwd_active(void);
void screen_cycle_bak(void);
void screen_cycle_bak_active(void);

#else
#	warning "included multiple times"
#endif
