INCLUDES = -I../missing -I../perl -I.. -I. -I../..

AM_CFLAGS = $(PERL_CFLAGS) $(DEBUG_CFLAGS) $(WARN_CFLAGS)
AM_CFLAGS += -DSYSTEM_PORKRC=\"$(pkgdatadir)/porkrc\"

noinst_LIBRARIES = libporkprefs.a

libporkprefs_a_SOURCES = \
	pork_acct_set.c \
	pork_conf.c \
	pork_imwindow_set.c \
	pork_input_set.c \
	pork_set.c \
	pork_set_global.c

noinst_HEADERS = \
	pork_acct_set.h \
	pork_conf.h \
	pork_imwindow_set.h \
	pork_input_set.h \
	pork_set.h \
	pork_set_global.h
