/*
** pork_set_global.h - /SET command implementation.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_SET_GLOBAL_H
#define __PORK_SET_GLOBAL_H

void init_global_prefs(struct screen *screen);

enum {
	OPT_AUTOSAVE = 0,
	OPT_BANNER,
	OPT_CMDCHARS,
	OPT_COLOR_BLIST_FOCUS,
	OPT_COLOR_BLIST_NOFOCUS,
	OPT_COLOR_BLIST_SELECTOR,
#define OPT_FORMAT_OFFSET OPT_FORMAT_ACTION_RECV
	OPT_FORMAT_ACTION_RECV,
	OPT_FORMAT_ACTION_RECV_STATUS,
	OPT_FORMAT_ACTION_SEND,
	OPT_FORMAT_ACTION_SEND_STATUS,
	OPT_FORMAT_BLIST,
	OPT_FORMAT_BLIST_GROUP,
	OPT_FORMAT_BLIST_IDLE,
	OPT_FORMAT_BLIST_WARN,
	OPT_FORMAT_CHAT_CREATE,
	OPT_FORMAT_CHAT_IGNORE,
	OPT_FORMAT_CHAT_INVITE,
	OPT_FORMAT_CHAT_JOIN,
	OPT_FORMAT_CHAT_KICK,
	OPT_FORMAT_CHAT_LEAVE,
	OPT_FORMAT_CHAT_MODE,
	OPT_FORMAT_CHAT_NICK,
	OPT_FORMAT_CHAT_QUIT,
	OPT_FORMAT_CHAT_RECV,
	OPT_FORMAT_CHAT_RECV_ACTION,
	OPT_FORMAT_CHAT_RECV_NOTICE,
	OPT_FORMAT_CHAT_SEND,
	OPT_FORMAT_CHAT_SEND_ACTION,
	OPT_FORMAT_CHAT_SEND_NOTICE,
	OPT_FORMAT_CHAT_TOPIC,
	OPT_FORMAT_CHAT_UNIGNORE,
	OPT_FORMAT_FILE_CANCEL_LOCAL,
	OPT_FORMAT_FILE_CANCEL_REMOTE,
	OPT_FORMAT_FILE_LOST,
	OPT_FORMAT_FILE_RECV_ACCEPT,
	OPT_FORMAT_FILE_RECV_ASK,
	OPT_FORMAT_FILE_RECV_COMPLETE,
	OPT_FORMAT_FILE_RECV_RESUME,
	OPT_FORMAT_FILE_SEND_ACCEPT,
	OPT_FORMAT_FILE_SEND_ASK,
	OPT_FORMAT_FILE_SEND_COMPLETE,
	OPT_FORMAT_FILE_SEND_RESUME,
	OPT_FORMAT_MSG_RECV,
	OPT_FORMAT_MSG_RECV_AUTO,
	OPT_FORMAT_MSG_RECV_STATUS,
	OPT_FORMAT_MSG_SEND,
	OPT_FORMAT_MSG_SEND_AUTO,
	OPT_FORMAT_MSG_SEND_STATUS,
	OPT_FORMAT_NOTICE_RECV,
	OPT_FORMAT_NOTICE_RECV_STATUS,
	OPT_FORMAT_NOTICE_SEND,
	OPT_FORMAT_NOTICE_SEND_STATUS,
	OPT_FORMAT_WARN_RECV,
	OPT_FORMAT_WARN_RECV_ANON,
	OPT_FORMAT_WARN_SEND,
	OPT_FORMAT_WARN_SEND_ANON,
	OPT_FORMAT_STATUS,
	OPT_FORMAT_STATUS_ACTIVITY,
	OPT_FORMAT_STATUS_CHAT,
	OPT_FORMAT_STATUS_HELD,
	OPT_FORMAT_STATUS_IDLE,
	OPT_FORMAT_STATUS_TIMESTAMP,
	OPT_FORMAT_STATUS_TYPING,
	OPT_FORMAT_STATUS_WARN,
	OPT_FORMAT_TIMESTAMP,
	OPT_PORK_DIR,
	OPT_RECURSIVE_EVENTS,
	OPT_TEXT_BLIST_GROUP_COLLAPSED,
	OPT_TEXT_BLIST_GROUP_EXPANDED,
	OPT_TEXT_BUDDY_ACTIVE,
	OPT_TEXT_BUDDY_AWAY,
	OPT_TEXT_BUDDY_IDLE,
	OPT_TEXT_NO_NAME,
	OPT_TEXT_NO_ROOM,
	OPT_TEXT_TYPING,
	OPT_TEXT_TYPING_PAUSED,
	GLOBAL_NUM_OPTS
};

#define DEFAULT_AUTOSAVE					0
#define DEFAULT_BANNER						"*** "
#define DEFAULT_CMDCHARS					'/'
#define DEFAULT_COLOR_BLIST_FOCUS			0x200400
#define DEFAULT_COLOR_BLIST_NOFOCUS			0x200500
#define DEFAULT_COLOR_BLIST_SELECTOR		0x202400
#define DEFAULT_FORMAT_ACTION_RECV			"[$T] %c* %W$N%x $M"
#define DEFAULT_FORMAT_ACTION_RECV_STATUS	"[$T] %C>%c* %W$N%D(%C$h%D)%x $M"
#define DEFAULT_FORMAT_ACTION_SEND			"[$T] %c* %W$N%x $M"
#define DEFAULT_FORMAT_ACTION_SEND_STATUS	"[$T] %D-> %m$R %c* %W$N%x $M"
#define DEFAULT_FORMAT_BLIST				" $B $N$>$i"
#define DEFAULT_FORMAT_BLIST_GROUP			"($E) $N ($O/$T)"
#define DEFAULT_FORMAT_BLIST_IDLE			"%B$I"
#define DEFAULT_FORMAT_BLIST_WARN			"$W%%"
#define DEFAULT_FORMAT_CHAT_CREATE			"[$T] %W$N%x has %Gcreated %c$R %D(%x$U%D)"
#define DEFAULT_FORMAT_CHAT_IGNORE			"[$T] %W$N%x has %Gignored%x $D in %c$R"
#define DEFAULT_FORMAT_CHAT_INVITE			"[$T] %W$N%x has %Ginvited%x $D to %c$R%x %D(%x$M%D)"
#define DEFAULT_FORMAT_CHAT_JOIN			"[%YJ%D/%x$T%D/%C$N%D/%c$H%x]"
#define DEFAULT_FORMAT_CHAT_KICK			"[%RL%D/%r$D%D/%c$N%D/%c$M%x]"
#define DEFAULT_FORMAT_CHAT_LEAVE			"[%rL%D/%x$T%D/%c$N%D/%c$H%x]"
#define DEFAULT_FORMAT_CHAT_MODE			"[%MM%D/%m$T%D/%W$N%D/%C$M%x]"
#define DEFAULT_FORMAT_CHAT_NICK			"[%YN%D/%x$T%D/%m$N%W ->%M $D%x]"
#define DEFAULT_FORMAT_CHAT_QUIT			"[%BQ%D/%x$T%D/%B$N%D/%b$H%D/%x$M%x]"
#define DEFAULT_FORMAT_CHAT_RECV			"[$T] %b<%x$N%b>%x $M"
#define DEFAULT_FORMAT_CHAT_RECV_ACTION		"[$T] %c* %W$N%x $M"
#define DEFAULT_FORMAT_CHAT_RECV_NOTICE		"[$T] %D-%B$N%Y:%b$C%D-%x $M"
#define DEFAULT_FORMAT_CHAT_SEND			"[$T] $C%m>%x $M"
#define DEFAULT_FORMAT_CHAT_SEND_ACTION		"[$T] %c* %W$N%x $M"
#define DEFAULT_FORMAT_CHAT_SEND_NOTICE		"[$T] %D-> -%c$N%Y:%b$C%D-%x $M"
#define DEFAULT_FORMAT_CHAT_TOPIC			"[%GT%D/%g$T%D/%W$N%D/%x$M%x]"
#define DEFAULT_FORMAT_CHAT_UNIGNORE		"%W$N%x has %Gunignored%x $D in %c$R"
#define DEFAULT_FORMAT_FILE_CANCEL_LOCAL	"[$T] $U has canceled file transfer $I with $u for $N. $s bytes were sent"
#define DEFAULT_FORMAT_FILE_CANCEL_REMOTE	"[$T] $u has canceled file transfer $I with $U for $N. $s bytes were sent"
#define DEFAULT_FORMAT_FILE_LOST			"[$T] File transfer $I with $u for $N has been lost. $s bytes were sent"
#define DEFAULT_FORMAT_FILE_RECV_ACCEPT		"[$T] $U has accepted $N from $u. Beginning transfer"
#define DEFAULT_FORMAT_FILE_RECV_ASK		"[$T] $u [$F:$p] offered $N ($S bytes) to $U. Type /file get $I to accept it"
#define DEFAULT_FORMAT_FILE_RECV_COMPLETE	"[$T] Transfer $I has completed successfully: $N ($s bytes) was received by $U from $u in $t seconds ($R KB/s)"
#define DEFAULT_FORMAT_FILE_RECV_RESUME		"[$T] $U has accepted $N from $u, resuming at byte $O"
#define DEFAULT_FORMAT_FILE_SEND_ACCEPT		"[$T] $u [$F:$p] has accepted $N from $U. Beginning transfer"
#define DEFAULT_FORMAT_FILE_SEND_ASK		"[$T] Sent request to send $N ($S bytes) from $u to $U, waiting for reply"
#define DEFAULT_FORMAT_FILE_SEND_COMPLETE	"[$T] Transfer $I has completed successfully: $N ($s bytes) was sent by $U to $u in $T seconds ($R KB/s)"
#define DEFAULT_FORMAT_FILE_SEND_RESUME		"[$T] $u [$F:$p] has accepted $N from $U, resuming at byte $O"
#define DEFAULT_FORMAT_MSG_RECV				"[$T] %m<%x$N%m>%x $M"
#define DEFAULT_FORMAT_MSG_RECV_AUTO			"[$T] %Ca%cuto%D-%Cr%cesponse%x from %W$N%D:%x $M"
#define DEFAULT_FORMAT_MSG_RECV_STATUS		"[$T] %D*%M$N%D(%m$h%D)*%x $M"
#define DEFAULT_FORMAT_MSG_SEND				"[$T] %c>%x $M"
#define DEFAULT_FORMAT_MSG_SEND_AUTO		"[$T] %Ca%cuto%D-%Cr%cesponse%x to %W$R%D:%x $M"
#define DEFAULT_FORMAT_MSG_SEND_STATUS		"[$T] %D->*%m$R%D*%x $M"
#define DEFAULT_FORMAT_NOTICE_RECV			"[$T] %D-%B$N%D-%x $M"
#define DEFAULT_FORMAT_NOTICE_RECV_STATUS	"[$T] %D-%B$N%D(%c$h%D)-%x $M"
#define DEFAULT_FORMAT_NOTICE_SEND			"[$T] %D-> -%c$R%D-%x $M"
#define DEFAULT_FORMAT_NOTICE_SEND_STATUS	"[$T] %D-> -%c$R%D-%x $M"
#define DEFAULT_FORMAT_STATUS				"%d,w$T$n [$z$c]$A$Y$H $>$I$W%d,w$S [$!]"
#define DEFAULT_FORMAT_STATUS_ACTIVITY		" %w,d{$A}%d,w"
#define DEFAULT_FORMAT_STATUS_CHAT			"%d,w$T$@$n (+$u) [$z$c (+$M)]$A$Y$H $>$I$W%d,w$S [$!]"
#define DEFAULT_FORMAT_STATUS_HELD			" <%g,w$H%d,w>"
#define DEFAULT_FORMAT_STATUS_IDLE			"%d,w (%D,widle: $i%d,w)"
#define DEFAULT_FORMAT_STATUS_TIMESTAMP		"[$H:$M] "
#define DEFAULT_FORMAT_STATUS_TYPING		" (%b,w$Y%d,w)"
#define DEFAULT_FORMAT_STATUS_WARN			"%d,w [%r,w$w%%%d,w]"
#define DEFAULT_FORMAT_TIMESTAMP			"$H:$M"
#define DEFAULT_FORMAT_WARN_RECV			"[$T] %R$N%r has been warned by %R$U%r.%x Warning level is now %W$W%%"
#define DEFAULT_FORMAT_WARN_RECV_ANON		"[$T] %R$N%r has been warned anonymously.%x Warning level is now %W$W%%"
#define DEFAULT_FORMAT_WARN_SEND			"[$T] %R$U%r has warned %R$N"
#define DEFAULT_FORMAT_WARN_SEND_ANON		"[$T] %R$U%r has warned %R$N%x anonymously"
#define DEFAULT_PORK_DIR					"~/.pork/"
#define DEFAULT_RECURSIVE_EVENTS			0
#define DEFAULT_TEXT_BLIST_GROUP_EXPANDED	"%B+%x"
#define DEFAULT_TEXT_BLIST_GROUP_COLLAPSED	"%R-%x"
#define	DEFAULT_TEXT_BUDDY_ACTIVE			"%G*%x"
#define DEFAULT_TEXT_BUDDY_AWAY				"%r*%x"
#define DEFAULT_TEXT_BUDDY_IDLE				"%Y*%x"
#define DEFAULT_TEXT_NO_NAME				"<not specified>"
#define DEFAULT_TEXT_NO_ROOM				":(not joined)"
#define DEFAULT_TEXT_TYPING					"Typing"
#define DEFAULT_TEXT_TYPING_PAUSED			"Typing [paused]"

#else
#	warning "included multiple times"
#endif
