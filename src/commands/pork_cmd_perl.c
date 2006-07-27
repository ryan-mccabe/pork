/*
** pork_cmd_perl.c - /perl commands.
** Copyright (C) 2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <sys/time.h>
#include <ncurses.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_events.h>
#include <pork_acct.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_command.h>
#include <pork_perl.h>

USER_COMMAND(cmd_perl_eval) {
	size_t num_args = 0;
	char *p = args;
	char **perl_args;
	char *function;
	char *orig;
	size_t i = 0;

	if (blank_str(args))
		return;

	p = args;
	while (*p == ' ')
		p++;

	if (*p == '$')
		p++;
	args = p;
	orig = p;

	function = args;
	p = strchr(function, '(');
	if (p != NULL) {
		*p++ = '\0';
		args = p;
	}

	p = strchr(function, ' ');
	if (p != NULL) {
		*p = '\0';
		args = p + 1;
	}

	if (args == orig) {
		execute_perl(function, NULL);
		return;
	}

	p = args;
	while ((p = strchr(p, ',')) != NULL) {
		++num_args;
		++p;
	}
	num_args += 2;

	p = strchr(args, ')');
	if (p != NULL)
		*p = '\0';

	perl_args = xcalloc(num_args, sizeof(char *));

	while ((p = strsep(&args, ",")) != NULL) {
		char *s;

		while (*p == ' ' || *p == '\t')
			p++;

		s = strrchr(p, ' ');
		if (s != NULL && blank_str(s)) {
			while (*s == ' ')
				s--;
			s[1] = '\0';
		}

		perl_args[i++] = p;
	}

	perl_args[i] = NULL;

	execute_perl(function, perl_args);
	free(perl_args);
}

/*
** This destroys the perl state.
** It has the effect of unloads all scripts. All scripts should
** be catching the EVENT_UNLOAD event and doing any necessary cleanup
** when it happens.
*/

USER_COMMAND(cmd_perl_dump) {
	dlist_t *cur;

	for (cur = globals.acct_list ; cur != NULL ; cur = cur->next) {
		struct pork_acct *a = cur->data;
		event_generate(acct->events, EVENT_UNLOAD, a->refnum);
	}

	perl_destroy();
	perl_init();
}

USER_COMMAND(cmd_perl_load) {
	int ret;
	char buf[PATH_MAX];

	if (args == NULL)
		return;

	expand_path(args, buf, sizeof(buf));
	ret = perl_load_file(buf);
	if (ret != 0)
		screen_err_msg(_("Error: The file \"%s\" couldn't be loaded"), buf);
}

static struct command perl_command[] = {
	{ "",				cmd_perl_eval		},
    { "dump",			cmd_perl_dump		},
	{ "eval",			cmd_perl_eval		},
    { "load",			cmd_perl_load		}
};

struct command_set perl_set = {
	perl_command,
	array_elem(perl_command),
	"perl "
};
