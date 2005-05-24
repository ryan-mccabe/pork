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
#include <stdarg.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_color.h>
#include <pork_set.h>
#include <pork_imwindow.h>
#include <pork_imwindow_set.h>
#include <pork_acct.h>
#include <pork_acct_set.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

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
	struct imwindow *win;

	win = va_arg(ap, struct imwindow *);
	if (win != NULL)
		swindow_set_wordwrap(&win->swindow);
}

static const struct pork_pref win_pref_list[] = {
	{	.name = "ACTIVITY_TYPES",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "BEEP",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "BEEP_MAX",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "BEEP_ON_OUTPUT",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "HISTORY_LEN",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
		.updated = optchanged_histlen
	},{	.name = "LOG",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
		.updated = optchanged_log
	},{	.name = "LOG_TYPES",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "LOGFILE",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
		.updated = optchanged_logfile
	},{	.name = "PRIVATE_INPUT",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
		.updated = optchanged_priv_input
	},{	.name = "PROMPT_STR",
		.type = OPT_TYPE_STR,
		.set = opt_set_str,
		.updated = optchanged_prompt,
	},{	.name = "SCROLL_ON_INPUT",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "SCROLL_ON_OUTPUT",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "SCROLLBUF_LEN",
		.type = OPT_TYPE_INT,
		.set = opt_set_int,
	},{	.name = "SHOW_BLIST",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
		.updated = optchanged_show_blist
	},{	.name = "SHOW_BUDDY_AWAY",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "SHOW_BUDDY_IDLE",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name ="SHOW_BUDDY_SIGNON",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
	},{	.name = "WORDWRAP",
		.type = OPT_TYPE_BOOL,
		.set = opt_set_bool,
		.updated = optchanged_wordwrap
	},{	.name = "WORDWRAP_CHAR",
		.type = OPT_TYPE_CHAR,
		.set = opt_set_char,
		.updated = optchanged_wordwrap
	}
};

static const struct pref_set win_pref_set = {
	.name = "win",
	.num_opts = WIN_NUM_OPTS,
	.prefs = win_pref_list
};

static pref_val_t win_default_pref_vals[] = {
	{	.pref_val.i = DEFAULT_WIN_ACTIVITY_TYPES,
	},{ .pref_val.b = DEFAULT_WIN_BEEP,
	},{ .pref_val.i = DEFAULT_WIN_BEEP_MAX,
	},{ .pref_val.b = DEFAULT_WIN_BEEP_ON_OUTPUT,
	},{ .pref_val.i = DEFAULT_WIN_HISTORY_LEN,
	},{ .pref_val.b = DEFAULT_WIN_LOG,
	},{ .pref_val.i = DEFAULT_WIN_LOG_TYPES,
	},{ .pref_val.s = DEFAULT_WIN_LOGFILE,
	},{ .pref_val.b = DEFAULT_WIN_PRIVATE_INPUT,
	},{ .pref_val.s = DEFAULT_WIN_PROMPT,
	},{ .pref_val.b = DEFAULT_WIN_SCROLL_ON_INPUT,
	},{ .pref_val.b = DEFAULT_WIN_SCROLL_ON_OUTPUT,
	},{ .pref_val.b = DEFAULT_WIN_SCROLLBUF_LEN,
	},{ .pref_val.b = DEFAULT_WIN_SHOW_BLIST,
	},{ .pref_val.b = DEFAULT_WIN_SHOW_BUDDY_AWAY,
	},{ .pref_val.b = DEFAULT_WIN_SHOW_BUDDY_IDLE,
	},{ .pref_val.b = DEFAULT_WIN_SHOW_BUDDY_SIGNON,
	},{ .pref_val.b = DEFAULT_WIN_WORDWRAP,
	},{ .pref_val.c = DEFAULT_WIN_WORDWRAP_CHAR,
	}
};

static struct pref_val win_defaults = {
	.set = &win_pref_set,
	.val = win_default_pref_vals,
};

int imwindow_init_prefs(struct imwindow *win) {
	struct pref_val *pref;

	pref = xmalloc(sizeof(*pref));
	pref->set = &win_pref_set;
	pref->val = xmalloc(sizeof(win_default_pref_vals));
	memcpy(pref->val, win_default_pref_vals, sizeof(win_default_pref_vals));

	if (win->target) {
		char buf[1024];
		char *p;
		char *log_dir;
		int ret;

		normalize(buf, win->target, sizeof(buf));
		while ((p = strchr(buf, '/')) != NULL)
			*p = '_';

		log_dir = opt_get_str(win->owner->prefs, ACCT_OPT_LOG_DIR);
		if (log_dir != NULL) {
			char logfile[4096];

			ret = snprintf(logfile, sizeof(logfile), "%s/%s.log", log_dir, buf);
			if (ret > 0 && (size_t) ret < sizeof(logfile))
				opt_set(pref, WIN_OPT_LOGFILE, logfile);
		}
	}

	win->prefs = pref;
	return (0);
}

inline struct pref_val *imwindow_get_default_prefs(void) {
	return (&win_defaults);
}
