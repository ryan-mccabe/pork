SUBDIRS = missing perl prefs protocols
bin_PROGRAMS = pork

if IRC_SUPPORT
irc_lib = protocols/irc/libirc.a
endif

if COMPILE_PERL
perl_lib = perl/libporkperl.a
endif

INCLUDES = -Imissing -Iperl -Iprefs -I.. -I.

AM_CFLAGS = -funsigned-char
AM_CFLAGS += -DHELP_PATH=\"$(pkgdatadir)/help\" -DSYSTEM_PORKRC=\"$(pkgdatadir)/porkrc\"
AM_CFLAGS += $(PERL_CFLAGS) $(DEBUG_CFLAGS)

pork_LDADD = prefs/libporkprefs.a missing/libmissing.a protocols/aim/libaim.a $(irc_lib) $(perl_lib) -lncurses $(PERL_LIBS) $(DEBUG_LIBS)

noinst_HEADERS = \
	pork.h				\
	pork_acct.h			\
	pork_alias.h		\
	pork_bind.h			\
	pork_buddy.h		\
	pork_buddy_list.h	\
	pork_chat.h			\
	pork_color.h		\
	pork_command.h		\
	pork_command_defs.h	\
	pork_cstr.h			\
	pork_events.h		\
	pork_format.h		\
	pork_help.h			\
	pork_html.h			\
	pork_imwindow.h		\
	pork_imsg.h			\
	pork_inet.h			\
	pork_input.h		\
	pork_io.h			\
	pork_list.h			\
	pork_misc.h			\
	pork_msg.h			\
	pork_opt.h			\
	pork_proto.h		\
	pork_queue.h		\
	pork_screen.h		\
	pork_screen_io.h	\
	pork_slist.h		\
	pork_status.h		\
	pork_swindow.h		\
	pork_timer.h		\
	pork_transfer.h		\
	pork_util.h

pork_SOURCES = \
	pork.c				\
	pork_acct.c			\
	pork_alias.c		\
	pork_bind.c			\
	pork_buddy.c		\
	pork_buddy_list.c	\
	pork_chat.c			\
	pork_color.c		\
	pork_command.c		\
	pork_cstr.c			\
	pork_events.c		\
	pork_format.c		\
	pork_help.c			\
	pork_html.c			\
	pork_imwindow.c		\
	pork_imsg.c			\
	pork_inet.c			\
	pork_input.c		\
	pork_io.c			\
	pork_list.c			\
	pork_misc.c			\
	pork_msg.c			\
	pork_opt.c			\
	pork_proto.c		\
	pork_queue.c		\
	pork_screen.c		\
	pork_screen_io.c	\
	pork_slist.c		\
	pork_status.c		\
	pork_swindow.c		\
	pork_timer.c		\
	pork_transfer.c		\
	pork_util.c
