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

static void scrollbuf_len_update(struct pref_val *pref, va_list ap) {
#if 0
	u_int32_t scrollbuf_len;

	scrollbuf_len = opt_get_int(pref, OPT_SCROLLBUF_LEN);
	if (scrollbuf_len < cur_window()->swindow.rows)
		SET_INT(pref->val[OPT_SCROLLBUF_LEN], cur_window()->swindow.rows);
#endif
}

static void opt_changed_prompt(struct pref_val *pref, va_list ap __notused) {
#if 0
	input_set_prompt(&screen.input, opt_get_str(pref, OPT_PROMPT));
#endif
}

static void optchanged_histlen(struct pref_val *pref, va_list ap) {
#if 0
	u_int32_t new_len;
	struct imwindow *win = va_arg(ap, struct imwindow *);

	new_len = optget_int(imwindow->opts, WIN_OPT_HISTORY_LEN);
	imwindow->input->history_len = new_len;
	input_history_prune(imwindow->input);
#endif
}

static void optchanged_log(struct pref_val *pref, va_list ap) {
#if 0
	u_int32_t new_val;

	new_val = optget_bool(imwindow->opts, WIN_OPT_LOG);
	if (new_val != imwindow->swindow.logged) {
		if (new_val == 0)
			swindow_end_log(&imwindow->swindow);
		else {
			if (swindow_set_log(&imwindow->swindow) == -1)
				imwindow->opts[WIN_OPT_LOG].b = 0;
		}
	}
#endif
}

static void optchanged_logfile(struct pref_val *pref, va_list ap) {
#if 0
	swindow_set_logfile(&imwindow->swindow,
		optget_str(imwindow->opts, WIN_OPT_LOGFILE));
#endif
}

static void optchanged_priv_input(struct pref_val *pref, va_list ap) {
#if 0
	u_int32_t new_val;

	new_val = optget_bool(imwindow->opts, WIN_OPT_PRIVATE_INPUT);
	imwindow_set_priv_input(imwindow, new_val);
#endif
}

static void optchanged_prompt(struct pref_val *pref, va_list ap) {
}

static void optchanged_scrollbuf_len(struct pref_val *pref, va_list ap) {
#if 0
	u_int32_t new_len;

	new_len = optget_int(imwindow->opts, WIN_OPT_SCROLLBUF_LEN);
	if (new_len < imwindow->swindow.rows) {
		imwindow->opts[WIN_OPT_SCROLLBUF_LEN].i = imwindow->swindow.rows;
		imwindow->swindow.scrollbuf_max = imwindow->swindow.rows;
	} else
		imwindow->swindow.scrollbuf_max = new_len;

	swindow_prune(&imwindow->swindow);
#endif
}

static void optchanged_scroll_on_output(struct pref_val *pref, va_list ap) {
#if 0
	u_int32_t new_val;

	new_val = optget_bool(imwindow->opts, WIN_OPT_SCROLL_ON_OUTPUT);
	imwindow->swindow.scroll_on_output = new_val;
#endif
}

static void optchanged_scroll_on_input(struct pref_val *pref, va_list ap) {
#if 0
	u_int32_t new_val;

	new_val = optget_bool(imwindow->opts, WIN_OPT_SCROLL_ON_INPUT);
	imwindow->swindow.scroll_on_input = new_val;
#endif
}

static void optchanged_show_blist(struct pref_val *pref, va_list ap) {
#if 0
	u_int32_t new_val;
	struct pork_acct *acct;

	acct = imwindow->owner;
	if (acct->blist == NULL)
		return;

	new_val = optget_bool(imwindow->opts, WIN_OPT_SHOW_BLIST);
	if (new_val == imwindow->blist_visible)
		return;

	if (new_val)
		imwindow_blist_show(imwindow);
	else
		imwindow_blist_hide(imwindow);
#endif
}

static void optchanged_wordwrap(struct pref_val *pref, va_list ap) {
#if 0
	swindow_set_wordwrap(&imwindow->swindow,
		optget_bool(imwindow->opts, WIN_OPT_WORDWRAP));
#endif
}

void optinit(struct imwindow *imwindow, const char *target) {
#if 0
	char nnick[NUSER_LEN];
	char buf[NUSER_LEN + 256];
	char *pork_dir;
	char *p;
	pref_val_t *wopt = imwindow->opts;

	memset(wopt, 0, sizeof(pref_val_t) * WIN_NUM_OPTS);

	wopt[WIN_OPT_ACTIVITY_TYPES].i = opt_get_int(OPT_ACTIVITY_TYPES);
	wopt[WIN_OPT_BEEP_ON_OUTPUT].b = opt_get_bool(OPT_BEEP_ON_OUTPUT);
	wopt[WIN_OPT_HISTORY_LEN].i = opt_get_int(OPT_HISTORY_LEN);
	wopt[WIN_OPT_LOG].b = opt_get_bool(OPT_LOG);
	wopt[WIN_OPT_LOG_TYPES].i = opt_get_int(OPT_LOG_TYPES);
	wopt[WIN_OPT_PRIVATE_INPUT].b = opt_get_bool(OPT_PRIVATE_INPUT);
	wopt[WIN_OPT_SCROLL_ON_INPUT].b = opt_get_bool(OPT_SCROLL_ON_INPUT);
	wopt[WIN_OPT_SCROLL_ON_OUTPUT].b = opt_get_bool(OPT_SCROLL_ON_OUTPUT);
	wopt[WIN_OPT_SCROLLBUF_LEN].i = opt_get_int(OPT_SCROLLBUF_LEN);
	wopt[WIN_OPT_SHOW_BLIST].b = opt_get_bool(OPT_SHOW_BLIST);
	wopt[WIN_OPT_WORDWRAP].b = opt_get_bool(OPT_WORDWRAP);
	wopt[WIN_OPT_WORDWRAP_CHAR].c = opt_get_char(OPT_WORDWRAP_CHAR);

	normalize(nnick, target, sizeof(nnick));
	while ((p = strchr(nnick, '/')) != NULL)
		*p = '_';

	pork_dir = opt_get_str(OPT_PORK_DIR);

	snprintf(buf, sizeof(buf), "%s/%s/logs/%s.log",
		pork_dir, imwindow->owner->username, nnick);

	wopt[WIN_OPT_LOGFILE].s = xstrdup(buf);
#endif
}

static void optdestroy(struct imwindow *imwindow) {
#if 0
	free(imwindow->opts[WIN_OPT_LOGFILE].s);
#endif
}

/*
** Name:		The name of the window specific option.
** Type:		The type of the option (boolean, string, int, char)
** Set func:	The function used to set the option.
** Change func:	The function to be called when the option changes.
*/

static const struct pork_pref win_pref_list[] = {
	{	"ACTIVITY_TYPES",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	},{	"BEEP",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{ "BEEP_MAX",
		OPT_TYPE_INT,
		opt_set_int,
		NULL,
	},{	"BEEP_ON_OUTPUT",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL
	},{	"HISTORY_LEN",
		OPT_TYPE_INT,
		opt_set_int,
		optchanged_histlen
	},{	"LOG",
		OPT_TYPE_BOOL,
		opt_set_bool,
		optchanged_log
	},{	"LOG_TYPES",
		OPT_TYPE_INT,
		opt_set_int,
		NULL
	},{	"LOGFILE",
		OPT_TYPE_STR,
		opt_set_str,
		optchanged_logfile
	},{	"PRIVATE_INPUT",
		OPT_TYPE_BOOL,
		opt_set_bool,
		optchanged_priv_input
	},{ "PROMPT",
		OPT_TYPE_STR,
		opt_set_str,
		optchanged_prompt,
	},{	"SCROLL_ON_INPUT",
		OPT_TYPE_BOOL,
		opt_set_bool,
		optchanged_scroll_on_input
	},{	"SCROLL_ON_OUTPUT",
		OPT_TYPE_BOOL,
		opt_set_bool,
		optchanged_scroll_on_output
	},{	"SCROLLBUF_LEN",
		OPT_TYPE_INT,
		opt_set_int,
		optchanged_scrollbuf_len
	},{	"SHOW_BLIST",
		OPT_TYPE_BOOL,
		opt_set_bool,
		optchanged_show_blist
	},{ "SHOW_BUDDY_AWAY",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL,
	},{ "SHOW_BUDDY_IDLE",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL,
	},{ "SHOW_BUDDY_SIGNOFF",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL,
	},{ "SHOW_BUDDY_SIGNON",
		OPT_TYPE_BOOL,
		opt_set_bool,
		NULL,
	},{	"WORDWRAP",
		OPT_TYPE_BOOL,
		opt_set_bool,
		optchanged_wordwrap
	},{	"WORDWRAP_CHAR",
		OPT_TYPE_CHAR,
		opt_set_char,
		optchanged_wordwrap
	}
};

static const struct pref_set win_pref_set = {
	.name = "win",
	.num_opts = WIN_NUM_OPTS,
	.prefs = win_pref_list
};


static pref_val_t win_default_pref_vals[] = {
	{	.pref_val.i = DEFAULT_WIN_ACTIVITY_TYPES,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_BEEP,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_WIN_BEEP_MAX,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_BEEP_ON_OUTPUT,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_WIN_HISTORY_LEN,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_LOG,
		.dynamic = 0
	},{	.pref_val.i = DEFAULT_WIN_LOG_TYPES,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_WIN_LOGFILE,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_PRIVATE_INPUT,
		.dynamic = 0
	},{	.pref_val.s = DEFAULT_WIN_PROMPT,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_SCROLL_ON_INPUT,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_SCROLL_ON_OUTPUT,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_SCROLLBUF_LEN,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_SHOW_BLIST,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_SHOW_BUDDY_AWAY,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_SHOW_BUDDY_IDLE,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_SHOW_BUDDY_SIGNOFF,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_SHOW_BUDDY_SIGNON,
		.dynamic = 0
	},{	.pref_val.b = DEFAULT_WIN_WORDWRAP,
		.dynamic = 0
	},{	.pref_val.c = DEFAULT_WIN_WORDWRAP_CHAR,
		.dynamic = 0
	}
};

static struct pref_val win_defaults = {
	.set = &win_pref_set,
	.val = win_default_pref_vals
};
