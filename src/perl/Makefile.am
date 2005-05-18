INCLUDES = -I. -I.. -I../.. -I../missing -I../prefs
AM_CFLAGS = $(DEBUG_CFLAGS) $(PERL_CFLAGS)

noinst_LIBRARIES = libporkperl.a

noinst_HEADERS = \
	pork_perl.h			\
	pork_perl_xs.h

if COMPILE_PERL
libporkperl_a_SOURCES  = \
		pork_perl.c \
		pork_perl_xs.c \
		pork_perl_blist.c \
		pork_perl_buddy.c \
		pork_perl_chat.c \
		pork_perl_event.c \
		pork_perl_input.c \
		pork_perl_stub.c \
		pork_perl_timer.c \
		pork_perl_win.c
else
libporkperl_a_SOURCES  = pork_perl_stub.c
endif
