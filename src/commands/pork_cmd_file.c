/*
** pork_cmd_file.c - /file commands
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
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
#include <sys/time.h>
#include <ncurses.h>

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_transfer.h>
#include <pork_screen_io.h>
#include <pork_command.h>

USER_COMMAND(cmd_file_cancel) {
	u_int32_t refnum;
	struct file_transfer *xfer;
	char *refnum_str;

	if (args == NULL || blank_str(args))
		return;

	refnum_str = strsep(&args, " ");
	if (refnum_str == NULL)
		return;

	if (!strcasecmp(refnum_str, "all")) {
		transfer_abort_all(acct, TRANSFER_DIR_ANY);
		return;
	}

	if (!strcasecmp(refnum_str, "send")) {
		transfer_abort_all(acct, TRANSFER_DIR_SEND);
		return;
	}

	if (!strcasecmp(refnum_str, "recv") || !strcasecmp(refnum_str, "receive")) {
		transfer_abort_all(acct, TRANSFER_DIR_RECV);
		return;
	}

	if (str_to_uint(args, &refnum) != 0) {
		screen_err_msg("Invalid file transfer refnum: %s", args);
		return;
	}

	xfer = transfer_find_refnum(acct, refnum);
	if (xfer == NULL) {
		screen_err_msg("Invalid file transfer refnum: %s", args);
		return;
	}

	if (transfer_cancel_local(xfer) != 0)
		screen_err_msg("Error canceling file transfer %s", args);
}

USER_COMMAND(cmd_file_list) {
	if (acct->transfer_list != NULL) {
		dlist_t *cur = acct->transfer_list;

		while (cur != NULL) {
			struct file_transfer *xfer = cur->data;

			/* XXX - make this a format str */
			screen_cmd_output("%u: %s %s [%s: %u/%u (%.02f%%) - %.04f KB/s]",
				xfer->refnum,
				xfer->peer_username, xfer->fname_local,
				transfer_status_str(xfer),
				(u_int32_t) xfer->bytes_sent, (u_int32_t) xfer->file_len,
				(float) xfer->bytes_sent / xfer->file_len * 100,
				transfer_avg_speed(xfer));

			cur = cur->next;
		}
	}
}

USER_COMMAND(cmd_file_get) {
	char *refnum_str;
	u_int32_t refnum;
	struct file_transfer *xfer;

	if (args == NULL || blank_str(args))
		return;

	refnum_str = strsep(&args, " ");
	if (refnum_str == NULL)
		return;

	if (!strcasecmp(refnum_str, "all")) {
		transfer_get_all(acct);
		return;
	}

	if (str_to_uint(refnum_str, &refnum) != 0) {
		screen_err_msg("Invalid file transfer refnum: %s", refnum_str);
		return;
	}

	xfer = transfer_find_refnum(acct, refnum);
	if (xfer == NULL) {
		screen_err_msg("Invalid file transfer refnum: %s", refnum_str);
		return;
	}

	if (args != NULL && blank_str(args))
		args = NULL;

	transfer_get(xfer, args);
}

USER_COMMAND(cmd_file_resume) {
}

USER_COMMAND(cmd_file_send) {
	char *dest;

	if (args == NULL)
		return;

	dest = strsep(&args, " ");
	if (dest == NULL || args == NULL) {
		screen_err_msg("You must specify a user and a filename");
		return;
	}

	transfer_send(acct, dest, args);
}

static struct command file_command[] = {
	{ "abort",				cmd_file_cancel		},
	{ "cancel",				cmd_file_cancel		},
	{ "get",				cmd_file_get		},
	{ "list",				cmd_file_list		},
	{ "resume",				cmd_file_resume		},
	{ "send",				cmd_file_send		},
};

struct command_set file_set = { file_command, array_elem(file_command), "file " };
