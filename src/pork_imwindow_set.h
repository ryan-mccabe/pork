/*
** pork_imwindow_set.h - /win SET command implementation.
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_IMWINDOW_SET_H
#define __PORK_IMWINDOW_SET_H

/*
** Per-window options.
*/

enum {
	WOPT_ACTIVITY_TYPES = 0,
	WOPT_BEEP_ON_OUTPUT,
	WOPT_HISTORY_LEN,
	WOPT_LOG,
	WOPT_LOG_TYPES,
	WOPT_LOGFILE,
	WOPT_PRIVATE_INPUT,
	WOPT_SCROLL_ON_INPUT,
	WOPT_SCROLL_ON_OUTPUT,
	WOPT_SCROLLBUF_LEN,
	WOPT_SHOW_BLIST,
	WOPT_WORDWRAP,
	WOPT_WORDWRAP_CHAR,
	WOPT_NUM_OPTS,
};

struct imwindow;

struct window_var {
	char *name;
	u_int32_t type;
	int (*set)(struct imwindow *, u_int32_t, char *);
	void (*updated)(struct imwindow *);
};

void wopt_init(struct imwindow *imwindow, const char *target);
void wopt_destroy(struct imwindow *imwindow);

void wopt_print_var(struct imwindow *win, int var, const char *text);
void wopt_print(struct imwindow *win);
int wopt_set(struct imwindow *imwindow, u_int32_t opt, char *args);
int wopt_find(const char *name);
int wopt_get_val(	struct imwindow *imwindow,
					const char *opt_name,
					char *buf,
					size_t len);

#define wopt_get_int(wopt, opt) ((wopt)[(opt)].i)
#define wopt_get_str(wopt, opt) ((wopt)[(opt)].s)
#define wopt_get_char(wopt, opt) ((wopt)[(opt)].c)
#define wopt_get_bool(wopt, opt) ((wopt)[(opt)].b)

#endif
