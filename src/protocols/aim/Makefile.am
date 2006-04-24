INCLUDES = -I. -I.. -I../.. -I../../.. -I../../missing -I../../prefs -Ilibfaim
AM_CFLAGS = $(DEBUG_CFLAGS) $(PERL_CFLAGS) $(WARN_CFLAGS)

SUBDIRS = libfaim

noinst_LIBRARIES = libaim.a

libaim_a_SOURCES = \
	pork_aim.c \
	pork_aim_cmd.c \
	pork_aim_conf.c \
	pork_aim_html.c	\
	pork_aim_proto.c \
	pork_aim_set.c

noinst_HEADERS = \
	pork_aim.h \
	pork_aim_cmd.h \
	pork_aim_conf.h \
	pork_aim_html.h	\
	pork_aim_proto.h \
	pork_aim_set.h
