if IRC_SUPPORT
irc_dir = irc
endif

if AIM_SUPPORT
aim_dir = aim
endif

INCLUDES = -I. -I.. -I../.. -I../missing
AM_CFLAGS = $(DEBUG_CFLAGS) $(PERL_CFLAGS) $(WARN_CFLAGS)

SUBDIRS = $(irc_dir) $(aim_dir)
