INCLUDES = -I. -I.. -I../.. -I../missing
AM_CFLAGS = $(DEBUG_CFLAGS) $(PERL_CFLAGS)

noinst_LIBRARIES = libporkperl.a

if COMPILE_PERL
pork_perl = pork_perl.c
pork_perl_xs = pork_perl_xs.c
else
pork_perl = pork_perl_stub.c
endif

noinst_HEADERS = \
	pork_perl.h			\
	pork_perl_xs.h

libporkperl_a_SOURCES = \
	$(pork_perl)		\
	$(pork_perl_xs)
