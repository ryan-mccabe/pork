/*
** pork_conf.c - pork's configuration parser.
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_buddy.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_alias.h>
#include <pork_screen_io.h>
#include <pork_screen.h>
#include <pork_command.h>
#include <pork_set.h>
#include <pork_imwindow_set.h>
#include <pork_acct_set.h>
#include <pork_set_global.h>
#include <pork_conf.h>

static void write_alias_line(void *data, void *filep) {
	struct alias *alias = data;
	FILE *fp = filep;

	fprintf(fp, "alias %s %s%s\n",
		alias->alias, alias->orig, (alias->args ? alias->args : ""));
}

static void write_bind_line(void *data, void *filep) {
	struct binding *binding = data;
	FILE *fp = filep;
	char key_name[32];

	bind_get_keyname(binding->key, key_name, sizeof(key_name));
	fprintf(fp, "bind %s %s\n", key_name, binding->binding);
}

static void write_bind_blist_line(void *data, void *filep) {
	struct binding *binding = data;
	FILE *fp = filep;
	char key_name[32];

	bind_get_keyname(binding->key, key_name, sizeof(key_name));
	fprintf(fp, "bind -buddy %s %s\n", key_name, binding->binding);
}

int read_conf(struct pork_acct *acct, const char *path) {
	FILE *fp;
	char buf[8192];
	u_int32_t line = 0;
	char cmdchar = opt_get_char(globals.prefs, OPT_CMDCHARS);

	fp = fopen(path, "r");
	if (fp == NULL) {
		if (errno != ENOENT)
			debug("fopen: %s: %s", path, strerror(errno));
		return (-1);
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *p;

		++line;

		p = strchr(buf, '\n');
		if (p == NULL) {
			debug("%s: line %u too long", path, line);
			fclose(fp);
			return (-1);
		}

		*p = '\0';

		while ((p = strchr(p, '\t')) != NULL)
			*p++ = ' ';

		p = buf;
		while (*p == cmdchar || *p == ' ' || *p == '\t')
			p++;

		if (*p == '#')
			continue;

		if (!blank_str(p))
			run_command(acct, p);
	}

	fclose(fp);
	return (0);
}

int write_global_conf(char *path) {
	char porkrc[4096];
	FILE *fp;
	int ret;

	if (path == NULL)
		return (-1);

	ret = snprintf(porkrc, sizeof(porkrc), "%s-TEMP", path);
	if (ret < 0 || (size_t) ret > sizeof(porkrc))
		return (-1);

	fp = fopen(porkrc, "w");
	if (fp == NULL) {
		debug("fopen: %s: %s", porkrc, strerror(errno));
		return (-1);
	}

	opt_write(globals.prefs, fp);
	fprintf(fp, "\n\n");

	hash_iterate(&globals.alias_hash, write_alias_line, fp);
	fprintf(fp, "\n\n");
	hash_iterate(&globals.binds.main.hash, write_bind_line, fp);
	fprintf(fp, "\n\n");
	hash_iterate(&globals.binds.blist.hash, write_bind_blist_line, fp);

	if (fchmod(fileno(fp), 0600) != 0) {
		debug("fchmod: %s: %s", porkrc, strerror(errno));
		fclose(fp);
		unlink(porkrc);
		return (-1);
	}

	if (rename(porkrc, path) != 0) {
		debug("rename: %s => %s: %s", path, porkrc, strerror(errno));
		unlink(porkrc);
		return (-1);
	}

	return (0);
}

static int read_acct_conf(struct pork_acct *acct, const char *filename) {
	FILE *fp;
	char buf[2048];
	int line = 0;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		if (errno != ENOENT) {
			debug("fopen: %s: %s", filename, strerror(errno));
			return (-1);
		}

		return (0);
	}

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *p;

		++line;

		p = strchr(buf, '\n');
		if (p == NULL) {
			debug("line %u too long", line);
			fclose(fp);
			return (-1);
		}

		*p = '\0';

		while ((p = strchr(p, '\t')) != NULL)
			*p++ = ' ';

		p = buf;
		if (*p == '#')
			continue;

		p = strchr(buf, ':');
		if (p == NULL)
			continue;
		*p++ = '\0';

		while (*p == ' ')
			p++;

		if (!strcasecmp(buf, "username")) {
			free(acct->username);
			acct->username = xstrdup(p);
		} else if (!strcasecmp(buf, "password")) {
			if (acct->passwd != NULL) {
				memset(acct->passwd, 0, strlen(acct->passwd));
				free(acct->passwd);
			}

			acct->passwd = xstrdup(p);
		} else if (!strcasecmp(buf, "profile")) {
			free(acct->profile);
			acct->profile = xstrdup(p);
		} else {
			screen_err_msg(_("Error: account config line %d: bad setting: %s"),
				line, buf);
		}
	}

	fclose(fp);
	return (0);
}

int read_user_config(struct pork_acct *acct) {
	char buf[PATH_MAX];
	char *pork_dir = opt_get_str(acct->prefs, ACCT_OPT_ACCT_DIR);

	if (acct == NULL || pork_dir == NULL)
		return (-1);

	snprintf(buf, sizeof(buf), "%s/porkrc", pork_dir);
	if (read_conf(acct, buf) != 0 && errno != ENOENT)
		screen_err_msg(_("There was an error reading your porkrc file"));

	snprintf(buf, sizeof(buf), "%s/account", pork_dir);
	if (read_acct_conf(acct, buf) != 0)
		screen_err_msg(_("Error: Can't read account config file, %s"), buf);

	return (0);
}

#if 0
int save_acct_conf(struct pork_acct *acct, char *filename) {
	char *fn;
	size_t len;
	FILE *fp;

	len = strlen(filename) + sizeof("-TEMP");
	fn = xmalloc(len);
	snprintf(fn, len, "%s-TEMP", filename);

	create_full_path(fn);
	fp = fopen(fn, "w");
	if (fp == NULL) {
		debug("fopen: %s: %s", fn, strerror(errno));
		free(fn);
		return (-1);
	}

	if (acct->username != NULL)
		fprintf(fp, "username: %s\n", acct->username);
	if (acct->profile != NULL)
		fprintf(fp, "profile: %s\n", acct->profile);
	if (opt_get_bool(acct->prefs, ACCT_OPT_SAVE_PASSWD) && acct->passwd != NULL)
		fprintf(fp, "password: %s\n", acct->passwd);

	fchmod(fileno(fp), 0600);
	fclose(fp);

	if (rename(fn, filename) != 0) {
		debug("rename: %s<=>%s: %s", fn, filename, strerror(errno));
		unlink(fn);
		free(fn);
		return (-1);
	}

	free(fn);
	return (0);
}

int save_user_config(struct pork_acct *acct) {
	char buf[PATH_MAX];
	char *pork_dir = opt_get_str(acct->prefs, ACCT_OPT_PORK_DIR);

	if (acct == NULL || pork_dir == NULL) {
		debug("acct=%p port_dir=%p", acct, pork_dir);
		return (-1);
	}

	snprintf(buf, sizeof(buf), "%s/buddy_list", pork_dir);
	if (save_buddy_list(acct, buf) != 0)
		screen_err_msg("There was an error writing your buddy list.");

	snprintf(buf, sizeof(buf), "%s/account", pork_dir);
	if (save_acct_conf(acct, buf) != 0)
		screen_err_msg("Error: Can't write account config file, %s.", buf);

	return (0);
}
#endif

int read_global_config(void) {
	char *pork_dir;
	char buf[4096];

	if (read_conf(globals.null_acct, SYSTEM_PORKRC) != 0)
		screen_err_msg(_("Error reading the system-wide porkrc file"));

	pork_dir = opt_get_str(globals.prefs, OPT_PORK_DIR);
	if (pork_dir == NULL)
		return (-1);

	snprintf(buf, sizeof(buf), "%s/porkrc", pork_dir);
	if (read_conf(globals.null_acct, buf) != 0 &&
		errno != ENOENT)
	{
		return (-1);
	}

	return (0);
}
