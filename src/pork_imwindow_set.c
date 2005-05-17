/*
** pork_imwindow_set.c - /win set command implementation.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
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
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_color.h>
#include <pork_set.h>
#include <pork_imwindow_set.h>
#include <pork_imwindow.h>
#include <pork_acct.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

static int wopt_set_bool(struct imwindow *imwindow, u_int32_t opt, char *args);
static int wopt_set_int(struct imwindow *imwindow, u_int32_t opt, char *args);
static int wopt_set_str(struct imwindow *imwindow, u_int32_t opt, char *args);
static int wopt_set_char(	struct imwindow *imwindow,
							u_int32_t opt, char *args) __notused;
static void wopt_changed_histlen(struct imwindow *imwindow);
static void wopt_changed_log(struct imwindow *imwindow);
static void wopt_changed_logfile(struct imwindow *imwindow);
static void wopt_changed_priv_input(struct imwindow *imwindow);
static void wopt_changed_scrollbuf_len(struct imwindow *imwindow);
static void wopt_changed_scroll_on_output(struct imwindow *imwindow);
static void wopt_changed_scroll_on_input(struct imwindow *imwindow);
static void wopt_changed_show_blist(struct imwindow *imwindow);
static void wopt_changed_wordwrap(struct imwindow *imwindow);

/*
** Name:		The name of the window specific option.
** Type:		The type of the option (boolean, string, int, char)
** Set func:	The function used to set the option.
** Change func:	The function to be called when the option changes.
*/

static struct window_var window_var[] = {
	{	"ACTIVITY_TYPES",
		OPT_INT,
		wopt_set_int,
		NULL
	},{	"BEEP_ON_OUTPUT",
		OPT_BOOL,
		wopt_set_bool,
		NULL
	},{	"HISTORY_LEN",
		OPT_INT,
		wopt_set_int,
		wopt_changed_histlen
	},{	"LOG",
		OPT_BOOL,
		wopt_set_bool,
		wopt_changed_log
	},{	"LOG_TYPES",
		OPT_INT,
		wopt_set_int,
		NULL
	},{	"LOGFILE",
		OPT_STR,
		wopt_set_str,
		wopt_changed_logfile
	},{	"PRIVATE_INPUT",
		OPT_BOOL,
		wopt_set_bool,
		wopt_changed_priv_input
	},{	"SCROLL_ON_INPUT",
		OPT_BOOL,
		wopt_set_bool,
		wopt_changed_scroll_on_input
	},{	"SCROLL_ON_OUTPUT",
		OPT_BOOL,
		wopt_set_bool,
		wopt_changed_scroll_on_output
	},{	"SCROLLBUF_LEN",
		OPT_INT,
		wopt_set_int,
		wopt_changed_scrollbuf_len
	},{	"SHOW_BLIST",
		OPT_BOOL,
		wopt_set_bool,
		wopt_changed_show_blist
	},{	"WORDWRAP",
		OPT_BOOL,
		wopt_set_bool,
		wopt_changed_wordwrap
	},{	"WORDWRAP_CHAR",
		OPT_CHAR,
		wopt_set_char,
		wopt_changed_wordwrap
	},
};

/*
** Print out the value of the specified option.
*/

void wopt_print_var(struct imwindow *imwindow, int i, const char *text) {
	switch (window_var[i].type) {
		case OPT_BOOL:
			screen_nocolor_msg("%s %s %s", window_var[i].name, text,
				(imwindow->opts[i].b ? "TRUE" : "FALSE"));
			break;

		case OPT_STR:
			if (imwindow->opts[i].s != NULL &&
				imwindow->opts[i].s[0] != '\0')
			{
				screen_nocolor_msg("%s %s \"%s\"", window_var[i].name,
					text, imwindow->opts[i].s);
			} else
				screen_nocolor_msg("%s is <UNSET>", window_var[i].name);
			break;

		case OPT_INT:
			screen_nocolor_msg("%s %s %d", window_var[i].name,
				text, imwindow->opts[i].i);
			break;

		case OPT_CHAR:
			if (isprint(imwindow->opts[i].c)) {
				screen_nocolor_msg("%s %s '%c'", window_var[i].name, text,
					imwindow->opts[i].c);
			} else {
				screen_nocolor_msg("%s %s 0x%x", window_var[i].name, text,
					imwindow->opts[i].c);
			}
			break;
	}
}

int wopt_get_val(	struct imwindow *imwindow,
					const char *opt_name,
					char *buf,
					size_t len)
{
	int i;

	i = wopt_find(opt_name);
	if (i == -1)
		return (-1);

	switch (window_var[i].type) {
		case OPT_BOOL:
			snprintf(buf, len, "%d", imwindow->opts[i].b);
			break;

		case OPT_STR:
			if (imwindow->opts[i].s == NULL)
				return (-1);
			xstrncpy(buf, imwindow->opts[i].s, len);
			break;

		case OPT_INT:
			snprintf(buf, len, "%d", imwindow->opts[i].i);
			break;

		case OPT_CHAR:
			snprintf(buf, len, "%c", imwindow->opts[i].c);
			break;
	}

	return (0);
}

/*
** Print the values of all the window-specific options.
*/

void wopt_print(struct imwindow *imwindow) {
	size_t i;

	for (i = 0 ; i < array_elem(window_var) ; i++)
		wopt_print_var(imwindow, i, "is set to");
}

static int wopt_compare(const void *l, const void *r) {
	const char *str = l;
	const struct window_var *wvar = r;

	return (strcasecmp(str, wvar->name));
}

/*
** Same as above, only for window-specific options.
*/

int wopt_find(const char *name) {
	struct window_var *wvar;
	u_int32_t offset;

	wvar = bsearch(name, window_var, array_elem(window_var),
				sizeof(struct window_var), wopt_compare);

	if (wvar == NULL)
		return (-1);

	offset = (long) wvar - (long) &window_var[0];
	return (offset / sizeof(struct window_var));
}

static void wopt_changed_histlen(struct imwindow *imwindow) {
	u_int32_t new_len;

	new_len = wopt_get_int(imwindow->opts, WOPT_HISTORY_LEN);
	imwindow->input->history_len = new_len;
	input_history_prune(imwindow->input);
}

static void wopt_changed_log(struct imwindow *imwindow) {
	u_int32_t new_val;

	new_val = wopt_get_bool(imwindow->opts, WOPT_LOG);
	if (new_val != imwindow->swindow.logged) {
		if (new_val == 0)
			swindow_end_log(&imwindow->swindow);
		else {
			if (swindow_set_log(&imwindow->swindow) == -1)
				imwindow->opts[WOPT_LOG].b = 0;
		}
	}
}

static void wopt_changed_logfile(struct imwindow *imwindow) {
	swindow_set_logfile(&imwindow->swindow,
		wopt_get_str(imwindow->opts, WOPT_LOGFILE));
}

static void wopt_changed_priv_input(struct imwindow *imwindow) {
	u_int32_t new_val;

	new_val = wopt_get_bool(imwindow->opts, WOPT_PRIVATE_INPUT);
	imwindow_set_priv_input(imwindow, new_val);
}

static void wopt_changed_scrollbuf_len(struct imwindow *imwindow) {
	u_int32_t new_len;

	new_len = wopt_get_int(imwindow->opts, WOPT_SCROLLBUF_LEN);
	if (new_len < imwindow->swindow.rows) {
		imwindow->opts[WOPT_SCROLLBUF_LEN].i = imwindow->swindow.rows;
		imwindow->swindow.scrollbuf_max = imwindow->swindow.rows;
	} else
		imwindow->swindow.scrollbuf_max = new_len;

	swindow_prune(&imwindow->swindow);
}

static void wopt_changed_scroll_on_output(struct imwindow *imwindow) {
	u_int32_t new_val;

	new_val = wopt_get_bool(imwindow->opts, WOPT_SCROLL_ON_OUTPUT);
	imwindow->swindow.scroll_on_output = new_val;
}

static void wopt_changed_scroll_on_input(struct imwindow *imwindow) {
	u_int32_t new_val;

	new_val = wopt_get_bool(imwindow->opts, WOPT_SCROLL_ON_INPUT);
	imwindow->swindow.scroll_on_input = new_val;
}

static void wopt_changed_show_blist(struct imwindow *imwindow) {
	u_int32_t new_val;
	struct pork_acct *acct;

	acct = imwindow->owner;
	if (acct->blist == NULL)
		return;

	new_val = wopt_get_bool(imwindow->opts, WOPT_SHOW_BLIST);
	if (new_val == imwindow->blist_visible)
		return;

	if (new_val)
		imwindow_blist_show(imwindow);
	else
		imwindow_blist_hide(imwindow);
}

static void wopt_changed_wordwrap(struct imwindow *imwindow) {
	swindow_set_wordwrap(&imwindow->swindow,
		wopt_get_bool(imwindow->opts, WOPT_WORDWRAP));
}

void wopt_init(struct imwindow *imwindow, const char *target) {
	char nnick[NUSER_LEN];
	char buf[NUSER_LEN + 256];
	char *pork_dir;
	char *p;
	pref_val_t *wopt = imwindow->opts;

	memset(wopt, 0, sizeof(pref_val_t) * WOPT_NUM_OPTS);

	wopt[WOPT_ACTIVITY_TYPES].i = opt_get_int(OPT_ACTIVITY_TYPES);
	wopt[WOPT_BEEP_ON_OUTPUT].b = opt_get_bool(OPT_BEEP_ON_OUTPUT);
	wopt[WOPT_HISTORY_LEN].i = opt_get_int(OPT_HISTORY_LEN);
	wopt[WOPT_LOG].b = opt_get_bool(OPT_LOG);
	wopt[WOPT_LOG_TYPES].i = opt_get_int(OPT_LOG_TYPES);
	wopt[WOPT_PRIVATE_INPUT].b = opt_get_bool(OPT_PRIVATE_INPUT);
	wopt[WOPT_SCROLL_ON_INPUT].b = opt_get_bool(OPT_SCROLL_ON_INPUT);
	wopt[WOPT_SCROLL_ON_OUTPUT].b = opt_get_bool(OPT_SCROLL_ON_OUTPUT);
	wopt[WOPT_SCROLLBUF_LEN].i = opt_get_int(OPT_SCROLLBUF_LEN);
	wopt[WOPT_SHOW_BLIST].b = opt_get_bool(OPT_SHOW_BLIST);
	wopt[WOPT_WORDWRAP].b = opt_get_bool(OPT_WORDWRAP);
	wopt[WOPT_WORDWRAP_CHAR].c = opt_get_char(OPT_WORDWRAP_CHAR);

	normalize(nnick, target, sizeof(nnick));
	while ((p = strchr(nnick, '/')) != NULL)
		*p = '_';

	pork_dir = opt_get_str(OPT_PORK_DIR);

	snprintf(buf, sizeof(buf), "%s/%s/logs/%s.log",
		pork_dir, imwindow->owner->username, nnick);

	wopt[WOPT_LOGFILE].s = xstrdup(buf);
}

static int wopt_set_bool(struct imwindow *imwindow, u_int32_t opt, char *args) {
	int val = opt_tristate(args);

	if (val == -1)
		return (-1);

	if (val != 2)
		imwindow->opts[opt].b = val;
	else
		imwindow->opts[opt].b = !imwindow->opts[opt].b;

	if (window_var[opt].updated != NULL)
		window_var[opt].updated(imwindow);

	return (0);
}

static int wopt_set_char(struct imwindow *imwindow, u_int32_t opt, char *args) {
	if (args == NULL || *args == '\0')
		return (-1);

	if (!strncasecmp(args, "0x", 2)) {
		u_int32_t temp;

		if (str_to_uint(args, &temp) == -1 || temp > 0xff)
			imwindow->opts[opt].c = *args;
		else
			imwindow->opts[opt].c = temp;
	} else
		imwindow->opts[opt].c = *args;

	if (window_var[opt].updated != NULL)
		window_var[opt].updated(imwindow);

	return (0);
}

static int wopt_set_int(struct imwindow *imwindow, u_int32_t opt, char *args) {
	u_int32_t num;

	if (args == NULL)
		return (-1);

	if (str_to_uint(args, &num) != 0)
		return (-1);

	imwindow->opts[opt].i = num;

	if (window_var[opt].updated != NULL)
		window_var[opt].updated(imwindow);

	return (0);
}

static int wopt_set_str(struct imwindow *imwindow, u_int32_t opt, char *args) {
	free(imwindow->opts[opt].s);

	if (args != NULL)
		imwindow->opts[opt].s = xstrdup(args);
	else
		imwindow->opts[opt].s = NULL;

	if (window_var[opt].updated != NULL)
		window_var[opt].updated(imwindow);

	return (0);
}

inline int wopt_set(struct imwindow *imwindow, u_int32_t opt, char *args) {
	struct window_var *var = &window_var[opt];

	return (var->set(imwindow, opt, args));
}

void wopt_destroy(struct imwindow *imwindow) {
	free(imwindow->opts[WOPT_LOGFILE].s);
}
