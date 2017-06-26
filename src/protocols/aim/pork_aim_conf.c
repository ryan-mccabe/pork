/*
** pork_aim_conf.c - /aim save support.
** Copyright (C) 2005-2006 Ryan McCabe <ryan@numb.org>
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
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_buddy.h>
#include <pork_set.h>
#include <pork_proto.h>
#include <pork_set_global.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_acct_set.h>
#include <pork_screen_io.h>

#include <pork_aim_conf.h>

/*
** I was going to make this xml until I noticed
** the size of libxml2 (almost 1mb).
*/

static int read_buddy_list(struct pork_acct *acct, const char *filename) {
	FILE *fp;
	char buf[1024];
	u_int32_t line = 0;
	struct bgroup *cur_group = NULL;
	struct buddy *cur_buddy = NULL;
	struct buddy_pref *pref = acct->buddy_pref;

	fp = fopen(filename, "r");
	if (fp == NULL) {
		if (errno != ENOENT) {
			screen_err_msg(_("Can't open buddy list: %s: %s"),
				filename, strerror(errno));
			return (-1);
		}

		return (0);
	}

	acct->report_idle = opt_get_bool(acct->prefs, ACCT_OPT_REPORT_IDLE);
	pref->privacy_mode = 1;

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		char *p;
		char type;

		++line;
		p = strchr(buf, '\n');
		if (p == NULL) {
			screen_err_msg(_("Invalid buddy list data at line %u"), line);
			fclose(fp);
			return (-1);
		}

		*p = '\0';
		p = buf;

		type = *p++;

		while (*p == ' ' || *p == '\t')
			p++;

		if (type == 'g') {
			cur_buddy = NULL;
			cur_group = group_add(acct, p);
		} else if (type == 'b') {
			char *alias;
			struct buddy *buddy;

			if (cur_group == NULL) {
				screen_err_msg(_("Invalid buddy list data at line %u"), line);
				fclose(fp);
				return (-1);
			}

			alias = strchr(p, ':');
			if (alias != NULL)
				*alias++ = '\0';

			buddy = buddy_add(acct, p, cur_group, 0);
			if (alias != NULL)
				buddy_alias(acct, buddy, alias, 0);

			cur_buddy = buddy;
		} else if (type == 'l') {
			u_int32_t last_seen;

			if (str_to_uint(p, &last_seen) != 0 || cur_buddy == NULL) {
				screen_err_msg("Invalid buddy list data at line %u", line);
				fclose(fp);
				return (-1);
			}

			cur_buddy->last_seen = last_seen;
		} else if (type == 'n') {
			cur_buddy->notify = 1;
		} else if (type == 'p') {
			buddy_add_permit(acct, p, 0);

			cur_buddy = NULL;
			cur_group = NULL;
		} else if (type == 'd') {
			buddy_add_block(acct, p, 0);

			cur_buddy = NULL;
			cur_group = NULL;
		} else if (type == 'm') {
			u_int32_t privacy_mode;

			if (str_to_uint(p, &privacy_mode) != 0 || privacy_mode > 5) {
				screen_err_msg(
					"%s: Invalid value for permit/deny mode at line %u",
					p, line);
				fclose(fp);
				return (-1);
			}

			pref->privacy_mode = privacy_mode;

			cur_buddy = NULL;
			cur_group = NULL;
		} else if (type == 'i') {
			u_int32_t report_idle_val;

			if (str_to_uint(p, &report_idle_val) != 0 || report_idle_val > 1) {
				screen_err_msg(
					"Invalid value for report idle mode at line %u", line);
				fclose(fp);
				return (-1);
			}

			acct->report_idle = report_idle_val;

			cur_buddy = NULL;
			cur_group = NULL;
		} else {
			screen_err_msg("Invalid buddy list data at line %u", line);
			fclose(fp);
			return (-1);
		}
	}

	fclose(fp);
	return (0);
}

static int save_buddy_list(struct pork_acct *acct, const char *filename) {
	char *fn;
	size_t len;
	FILE *fp;
	dlist_t *gcur;
	struct buddy_pref *pref = acct->buddy_pref;

	len = strlen(filename) + sizeof("-TEMP");
	fn = xmalloc(len);
	snprintf(fn, len, "%s-TEMP", filename);

	create_full_path(fn);
	fp = fopen(fn, "w");
	if (fp == NULL) {
		screen_err_msg("Can't open buddy list file for writing: %s",
			strerror(errno));
		free(fn);
		return (-1);
	}

	fprintf(fp, "m %u\n", pref->privacy_mode);
	fprintf(fp, "i %u\n", acct->report_idle);

	for (gcur = pref->group_list ; gcur != NULL ; gcur = gcur->next) {
		struct bgroup *gr = gcur->data;
		dlist_t *bcur;

		fprintf(fp, "g %s\n", gr->name);

		for (bcur = gr->member_list ; bcur != NULL ; bcur = bcur->next) {
			struct buddy *buddy = bcur->data;

			fprintf(fp, "b %s:%s\n", buddy->nname, buddy->name);

			if (buddy->last_seen != 0)
				fprintf(fp, "l %u\n", buddy->last_seen);

			fprintf(fp, "n %u\n", buddy->notify);
		}
	}

	for (gcur = pref->permit_list ; gcur != NULL ; gcur = gcur->next)
		fprintf(fp, "p %s\n", (char *) gcur->data);

	for (gcur = pref->block_list ; gcur != NULL ; gcur = gcur->next)
		fprintf(fp, "d %s\n", (char *) gcur->data);

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

int read_conf(struct pork_acct *acct) {
	return (-1);
}

int save_conf(struct pork_acct *acct) {
	return (-1);
}
