/*
** pork_imwindow.h - interface for manipulating conversation/chat/info windows.
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_IMWINDOW_H
#define __PORK_IMWINDOW_H

#define IMWINDOW(x)	((struct imwindow *) (x))

struct pork_acct;
struct imsg;

#include <pork_set.h>
#include <pork_swindow.h>

enum {
	WIN_TYPE_STATUS,
	WIN_TYPE_PRIVMSG,
	WIN_TYPE_CHAT
};

struct imwindow {
	struct swindow swindow;
	struct input *input;
	struct pork_acct *owner;
	struct key_binds *active_binds;
	char *target;
	char *name;
	void *data;
	u_int32_t refnum;
	u_int32_t type:2;
	u_int32_t typing:2;
	u_int32_t blist_visible:1;
	u_int32_t input_focus:1;
	u_int32_t ignore_activity:1;
	u_int32_t skip:1;
	pref_val_t opts[WOPT_NUM_OPTS];
};

struct imwindow *imwindow_new(	u_int32_t rows,
								u_int32_t cols,
								u_int32_t refnum,
								u_int32_t type,
								struct pork_acct *owner,
								char *target);

void imwindow_resize(	struct imwindow *imwindow,
						u_int32_t rows,
						u_int32_t cols);

int imwindow_set_priv_input(struct imwindow *imwindow, int val);
int imwindow_blist_refresh(struct imwindow *imwindow);
void imwindow_blist_show(struct imwindow *imwindow);
void imwindow_blist_hide(struct imwindow *imwindow);
void imwindow_blist_toggle(struct imwindow *imwindow);
void imwindow_blist_draw(struct imwindow *imwindow);
void imwindow_send_msg(struct imwindow *win);
void imwindow_recv_msg(struct imwindow *win);
int imwindow_bind_acct(struct imwindow *imwindow, u_int32_t refnum);
int imwindow_bind_next_acct(struct imwindow *imwindow);
int imwindow_refresh(struct imwindow *imwindow);
void imwindow_destroy(struct imwindow *imwindow);
void imwindow_switch_focus(struct imwindow *imwindow);
void imwindow_buffer_find(struct imwindow *imwindow, char *str, u_int32_t opt);

struct imwindow *imwindow_find_refnum(u_int32_t refnum);
struct imwindow *imwindow_find(struct pork_acct *owner, const char *target);
struct imwindow *imwindow_find_name(struct pork_acct *owner, const char *name);
struct imwindow *imwindow_find_chat_target(	struct pork_acct *owner,
											const char *target);

inline int imwindow_add(struct imwindow *imwindow,
						struct imsg *imsg,
						u_int32_t type);

inline int imwindow_ignore(struct imwindow *imwindow);
inline int imwindow_unignore(struct imwindow *imwindow);
inline int imwindow_skip(struct imwindow *imwindow);
inline int imwindow_unskip(struct imwindow *imwindow);
inline int imwindow_dump_buffer(struct imwindow *imwindow, char *file);
inline void imwindow_rename(struct imwindow *imwindow, char *new_name);
inline void imwindow_scroll_up(struct imwindow *imwindow);
inline void imwindow_scroll_down(struct imwindow *imwindow);
inline void imwindow_scroll_by(struct imwindow *imwindow, int lines);
inline void imwindow_scroll_page_up(struct imwindow *imwindow);
inline void imwindow_scroll_page_down(struct imwindow *imwindow);
inline void imwindow_scroll_start(struct imwindow *imwindow);
inline void imwindow_scroll_end(struct imwindow *imwindow);
inline void imwindow_clear(struct imwindow *imwindow);
inline void imwindow_erase(struct imwindow *imwindow);

#endif
