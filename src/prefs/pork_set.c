/*
** pork_set.c - Settings support
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
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_color.h>
#include <pork_set.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>

void opt_copy_pref_val(struct pref_val *pref, pref_val_t *val, size_t n) {
	u_int32_t i;
	u_int32_t elements;

	pref->val = xmalloc(n);
	memcpy(pref->val, val, n);

	elements = n / sizeof(val[0]);
	for (i = 0 ; i < elements ; i++) {
		if (pref->val[i].dynamic)
			pref->val[i].pref_val.s = xstrdup(pref->val[i].pref_val.s);
	}
}

int opt_get_val(struct pref_val *pref, const char *opt, char *buf, size_t len) {
	int ret;
	u_int32_t i;

	ret = opt_find(pref->set, opt, &i);
	if (ret == -1)
		return (-1);

	switch (pref_type(pref, i)) {
		case OPT_TYPE_BOOL:
			ret = snprintf(buf, len, "%d", opt_get_bool(pref, i));
			break;

		case OPT_TYPE_STR: {
			char *str;

			str = opt_get_str(pref, i);
			if (str == NULL)
				return (-1);
			ret = xstrncpy(buf, str, len);
			break;
		}

		case OPT_TYPE_INT:
			ret = snprintf(buf, len, "%d", opt_get_int(pref, i));
			break;

		case OPT_TYPE_CHAR:
			ret = snprintf(buf, len, "%c", opt_get_char(pref, i));
			break;

		case OPT_TYPE_COLOR:
			ret = color_get_str(opt_get_int(pref, i), buf, len);
			break;

		default:
			ret = -1;
			debug("Unknown set type");
			break;
	}

	if (ret < 0 || (size_t) ret >= len)
		return (-1);

	return (0);
}

static void opt_print_var(struct pref_val *pref, u_int32_t i, const char *text)
{
	switch (pref_type(pref, i)) {
		case OPT_TYPE_BOOL:
			screen_nocolor_msg("%s %s %s", pref_name(pref, i),
				text, (opt_get_bool(pref, i) ? "TRUE" : "FALSE"));
			break;

		case OPT_TYPE_STR: {
			char *str;

			str = opt_get_str(pref, i);
			if (str != NULL && str[0] != '\0') {
				screen_nocolor_msg("%s %s \"%s\"",
					pref_name(pref, i), text, str);
			} else
				screen_nocolor_msg(_("%s is [UNSET]"), pref_name(pref, i));

			break;
		}

		case OPT_TYPE_INT:
			screen_nocolor_msg("%s %s %d",
				pref_name(pref, i), text, opt_get_int(pref, i));
			break;

		case OPT_TYPE_CHAR:
			if (isprint(opt_get_char(pref, i))) {
				screen_nocolor_msg("%s %s '%c'",
					pref_name(pref, i), text, opt_get_char(pref, i));
			} else {
				screen_nocolor_msg("%s %s 0x%x",
					pref_name(pref, i), text, opt_get_char(pref, i));
			}
			break;

		case OPT_TYPE_COLOR: {
			char buf[128];

			color_get_str(opt_get_int(pref, i), buf, sizeof(buf));
			screen_nocolor_msg("%s %s %s", pref_name(pref, i), text, buf);
			break;
		}

		default:
			debug("Unknown set type");
			break;
	}
}

/*
** Print the values of all the pref variables.
*/

static void opt_print(struct pref_val *pref) {
	size_t i;

	for (i = 0 ; i < pref->set->num_opts ; i++)
		opt_print_var(pref, i, _("is set to"));
}

void opt_write(struct pref_val *pref, FILE *fp) {
	size_t i;

	for (i = 0 ; i < pref->set->num_opts ; i++) {
		if (pref->set->name != NULL)
			fprintf(fp, "%s ", pref->set->name);

		switch (pref_type(pref, i)) {
			case OPT_TYPE_BOOL:
				fprintf(fp, "set %s %s\n",
					pref_name(pref, i),
					(opt_get_bool(pref, i) ? "TRUE" : "FALSE"));
				break;

			case OPT_TYPE_STR: {
				char *str;

				str = opt_get_str(pref, i);
				if (str != NULL && str[0] != 0)
					fprintf(fp, "set %s %s\n", pref_name(pref, i), str);
				break;
			}

			case OPT_TYPE_INT:
				fprintf(fp, "set %s %d\n",
					pref_name(pref, i), opt_get_int(pref, i));
				break;

			case OPT_TYPE_CHAR:
				if (isprint(opt_get_char(pref, i))) {
					fprintf(fp, "set %s %c\n",
						pref_name(pref, i), opt_get_char(pref, i));
				} else {
					fprintf(fp, "set %s 0x%x\n",
						pref_name(pref, i), opt_get_char(pref, i));
				}
				break;

			case OPT_TYPE_COLOR: {
				char buf[128];

				color_get_str(opt_get_int(pref, i), buf, sizeof(buf));
				fprintf(fp, "set %s %s\n", pref_name(pref, i), buf);
				break;
			}

			default:
				debug("Unknown set type");
				break;
		}
	}
}

static int opt_compare(const void *l, const void *r) {
	const char *str = l;
	const struct pork_pref *ppref = r;

	return (strcasecmp(str, ppref->name));
}

/*
** Find the position of the pref option named @name
** in the pref option table.
*/

int opt_find(const struct pref_set *pref_set, const char *name, u_int32_t *opt)
{
	struct pork_pref *ppref;
	u_int32_t offset;

	ppref = bsearch(name, pref_set->prefs, pref_set->num_opts,
				sizeof(*ppref), opt_compare);

	if (ppref == NULL)
		return (-1);

	offset = (long) ppref - (long) pref_set->prefs;
	*opt = offset / sizeof(*ppref);
	return (0);
}

/*
** The values a boolean variable can accept are TRUE, FALSE and TOGGLE.
** TRUE is 1, FALSE is 0. TOGGLE flips the current value of the variable.
*/

int opt_tristate(char *args) {
	if (args == NULL)
		return (-1);

	if (!strcasecmp(args, "ON") ||
		!strcasecmp(args, "TRUE") ||
		!strcasecmp(args, "1"))
	{
		return (1);
	}

	if (!strcasecmp(args, "OFF") ||
		!strcasecmp(args, "FALSE") ||
		!strcasecmp(args, "0"))
	{
		return (0);
	}

	if (!strcasecmp(args, "TOGGLE"))
		return (2);

	return (-1);
}

int opt_set_bool(struct pref_val *pref, u_int32_t opt, char *args, va_list ap) {
	int val = opt_tristate(args);

	if (val == -1)
		return (-1);

	/* toggle */
	if (val == 2)
		SET_BOOL(pref->val[opt], !opt_get_bool(pref, opt));
	else
		SET_BOOL(pref->val[opt], val);

	if (pref->set->prefs[opt].updated != NULL)
		pref->set->prefs[opt].updated(pref, ap);

	return (0);
}

int opt_set_char(struct pref_val *pref, u_int32_t opt, char *args, va_list ap) {
	if (args == NULL || *args == '\0')
		return (-1);

	if (!strncasecmp(args, "0x", 2)) {
		u_int32_t temp;

		if (str_to_uint(args, &temp) == -1 || temp > 0xff)
			SET_CHAR(pref->val[opt], *args);
		else
			SET_CHAR(pref->val[opt], temp);
	} else
		SET_CHAR(pref->val[opt], *args);

	if (pref->set->prefs[opt].updated != NULL)
		pref->set->prefs[opt].updated(pref, ap);

	return (0);
}

int opt_set_int(struct pref_val *pref, u_int32_t opt, char *args, va_list ap) {
	u_int32_t num;

	if (args == NULL)
		return (-1);

	if (str_to_uint(args, &num) != 0)
		return (-1);

	SET_INT(pref->val[opt], num);

	if (pref->set->prefs[opt].updated != NULL)
		pref->set->prefs[opt].updated(pref, ap);

	return (0);
}

int opt_set_str(struct pref_val *pref, u_int32_t opt, char *args, va_list ap) {
	if (pref_needs_free(pref, opt))
		free(opt_get_str(pref, opt));

	if (args != NULL) {
		char *str = xstrdup(args);

		SET_STR(pref->val[opt], str);
		pref_needs_free(pref, opt) = 1;
	} else {
		SET_STR(pref->val[opt], NULL);
		pref_needs_free(pref, opt) = 0;
	}

	if (pref->set->prefs[opt].updated != NULL)
		pref->set->prefs[opt].updated(pref, ap);

	return (0);
}

int opt_set_color(struct pref_val *pref, u_int32_t opt, char *args, va_list ap)
{
	attr_t attr = 0;

	if (*args != '%') {
		if (color_parse_code(args, &attr) == -1)
			return (-1);
	} else {
		char buf[32];
		chtype ch[4];

		snprintf(buf, sizeof(buf), "%s ", args);
		if (plaintext_to_cstr(ch, array_elem(ch), buf, NULL) != 1)
			return (-1);

		attr = ch[0] & A_ATTRIBUTES;
	}

	SET_INT(pref->val[opt], attr);

	if (pref->set->prefs[opt].updated != NULL)
		pref->set->prefs[opt].updated(pref, ap);

	return (0);
}

int opt_set(struct pref_val *pref, u_int32_t opt, char *args, ...) {
	va_list ap;
	int ret;

	va_start(ap, args);
	ret = pref->set->prefs[opt].set(pref, opt, args, ap);
	va_end(ap);

	return (ret);
}

void opt_destroy(struct pref_val *pref) {
	size_t i;

	for (i = 0 ; i < pref->set->num_opts ; i++) {
		if (pref_needs_free(pref, i))
			free(opt_get_str(pref, i));
	}
}

int opt_set_var(struct pref_val *pref, char *args, ...) {
	char *var;
	char *value;
	u_int32_t opt;
	int ret;
	va_list ap;

	if (blank_str(args))
		return (-1);

	var = strsep(&args, " ");
	if (blank_str(var)) {
		opt_print(pref);
		return (-1);
	}

	strtoupper(var);
	ret = opt_find(pref->set, var, &opt);
	if (ret == -1) {
		if (pref->set->name == NULL)
			screen_err_msg(_("Unknown variable: %s"), var);
		else
			screen_err_msg(_("Unknown %s variable: %s"), pref->set->name, var);

		return (-1);
	}

	value = args;
	if (blank_str(value)) {
		opt_print_var(pref, opt, _("is set to"));
		return (0);
	}

	va_start(ap, args);
	ret = pref->set->prefs[opt].set(pref, opt, value, ap);
	va_end(ap);

	if (ret == -1) {
		screen_nocolor_msg(_("Bad argument for %s: %s"), var, value);
		return (-1);
	} else
		opt_print_var(pref, opt, _("set to"));

	return (0);
}
