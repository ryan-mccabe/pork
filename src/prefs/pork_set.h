/*
** pork_set.h - /SET command implementation.
** Copyright (C) 2002-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_SET_H
#define __PORK_SET_H

enum {
	OPT_TYPE_INT 		= 0,
	OPT_TYPE_CHAR 		= 1,
	OPT_TYPE_STR 		= 2,
	OPT_TYPE_FORMAT 	= 2,
	OPT_TYPE_BOOL 		= 3,
	OPT_TYPE_COLOR 		= 4
};

typedef struct {
	union {
		u_int32_t i;
		u_int32_t b;
		char c;
		char *s;
	} pref_val;
	char dynamic;
} pref_val_t;

struct pref_val {
	const struct pref_set *set;
	pref_val_t *val;
};

struct pref_set {
	char *name;
	size_t num_opts;
	const struct pork_pref *prefs;
};

struct pork_pref {
	char *name;
	u_int32_t type;
	int (*set)(struct pref_val *, u_int32_t, char *, va_list);
	void (*updated)(struct pref_val *, va_list);
};

int opt_set_bool(struct pref_val *pref, u_int32_t opt, char *args, va_list);
int opt_set_char(struct pref_val *pref, u_int32_t opt, char *args, va_list);
int opt_set_int(struct pref_val *pref, u_int32_t opt, char *args, va_list);
int opt_set_color(struct pref_val *pref, u_int32_t opt, char *args, va_list);
int opt_set_str(struct pref_val *pref, u_int32_t opt, char *args, va_list);
#define opt_set_format opt_set_str

void opt_destroy(struct pref_val *pref);
void opt_print_var(struct pref_val *pref, int var, const char *text);
void opt_print(struct pref_val *pref);
void opt_write(struct pref_val *pref, FILE *fp);

int opt_tristate(char *args);
int opt_set(struct pref_val *pref, u_int32_t opt, char *args, ...);
int opt_find(const struct pref_set *pref, const char *name);
int opt_get_val(struct pref_val *pref, const char *opt, char *buf, size_t len);

#define SET_STR(y,x)				((y).pref_val.s = (x))
#define SET_INT(y,x)				((y).pref_val.i = (x))
#define SET_CHAR(y,x)				((y).pref_val.c = (x))
#define SET_BOOL(y,x)				((y).pref_val.b = (x))

#define opt_get_int(pref,opt)		((pref)->val[(opt)].pref_val.i)
#define opt_get_color(pref,opt)		((pref)->val[(opt)].pref_val.i)
#define opt_get_bool(pref,opt)		((pref)->val[(opt)].pref_val.b)
#define opt_get_char(pref,opt)		((pref)->val[(opt)].pref_val.c)
#define opt_get_str(pref,opt)		((pref)->val[(opt)].pref_val.s)

#define pref_name(pref,opt)			((pref)->set->prefs[i].name)
#define pref_type(pref,opt)			((pref)->set->prefs[i].type)

#endif
