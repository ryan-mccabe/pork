INCLUDES = -I. -I.. -I../.. -I../../.. -I../../missing -I../../prefs
AM_CFLAGS = $(DEBUG_CFLAGS) $(PERL_CFLAGS) $(WARN_CFLAGS)

noinst_LIBRARIES = libirc.a

libirc_a_SOURCES =		\
	pork_irc.c			\
	pork_irc_cmd.c		\
	pork_irc_dcc.c		\
	pork_irc_format.c	\
	pork_irc_input.c	\
	pork_irc_output.c	\
	pork_irc_set.c

noinst_HEADERS = \
	pork_irc.h \
	pork_irc_cmd.h \
	pork_irc_dcc.h \
	pork_irc_set.h
