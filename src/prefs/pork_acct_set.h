/*
** pork_acct_set.h - /acct set command implementation.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_ACCT_SET_H
#define __PORK_ACCT_SET_H

int acct_init_prefs(struct pork_acct *acct);
struct pref_val *acct_get_default_prefs(void);

enum {
	ACCT_OPT_ACCT_DIR,
	ACCT_OPT_AUTO_RECONNECT,
	ACCT_OPT_AUTO_REJOIN,
	ACCT_OPT_AUTOSAVE,
	ACCT_OPT_AUTOSEND_AWAY,
	ACCT_OPT_CONNECT_TIMEOUT,
	ACCT_OPT_DOWNLOAD_DIR,
	ACCT_OPT_DUMP_MSGS_TO_STATUS,
	ACCT_OPT_IDLE_AFTER,
	ACCT_OPT_LOG_DIR,
	ACCT_OPT_LOGIN_ON_STARTUP,
	ACCT_OPT_RECONNECT_INTERVAL,
	ACCT_OPT_RECONNECT_MAX_INTERVAL,
	ACCT_OPT_RECONNECT_TRIES,
	ACCT_OPT_REPORT_IDLE,
	ACCT_OPT_SAVE_PASSWD,
	ACCT_OPT_SEND_REMOVES_AWAY,
	ACCT_OPT_TRANSFER_PORT_MAX,
	ACCT_OPT_TRANSFER_PORT_MIN,
	ACCT_NUM_OPTS
};

#define DEFAULT_ACCT_DIR						NULL
#define DEFAULT_ACCT_AUTO_RECONNECT				1
#define DEFAULT_ACCT_AUTO_REJOIN				1
#define DEFAULT_ACCT_AUTOSAVE					0
#define DEFAULT_ACCT_AUTOSEND_AWAY				0
#define DEFAULT_ACCT_CONNECT_TIMEOUT			180
#define DEFAULT_ACCT_DOWNLOAD_DIR				NULL
#define DEFAULT_ACCT_DUMP_MSGS_TO_STATUS		0
#define DEFAULT_ACCT_IDLE_AFTER					10
#define DEFAULT_ACCT_LOG_DIR					NULL
#define DEFAULT_ACCT_LOGIN_ON_STARTUP			1
#define DEFAULT_ACCT_RECONNECT_INTERVAL			15
#define DEFAULT_ACCT_RECONNECT_MAX_INTERVAL		600
#define DEFAULT_ACCT_RECONNECT_TRIES			10
#define DEFAULT_ACCT_REPORT_IDLE				1
#define DEFAULT_ACCT_SAVE_PASSWD				0
#define DEFAULT_ACCT_SEND_REMOVES_AWAY			1
#define DEFAULT_ACCT_TRANSFER_PORT_MAX			15000
#define DEFAULT_ACCT_TRANSFER_PORT_MIN			10000

#else
#	warning "included multiple times"
#endif
