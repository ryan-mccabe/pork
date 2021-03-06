/*
** pork.c - main function.
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <sys/signal.h>
#include <sys/ioctl.h>
#include <time.h>
#include <sys/time.h>

#ifdef HAVE_TERMIOS_H
#	include <termios.h>
#elif defined HAVE_SYS_TERMIOS_H
#	include <sys/termios.h>
#endif

#include <pork.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_set.h>
#include <pork_imsg.h>
#include <pork_swindow.h>
#include <pork_imwindow.h>
#include <pork_inet.h>
#include <pork_acct.h>
#include <pork_acct_set.h>
#include <pork_misc.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_events.h>
#include <pork_screen_io.h>
#include <pork_status.h>
#include <pork_alias.h>
#include <pork_opt.h>
#include <pork_conf.h>
#include <pork_color.h>
#include <pork_command.h>
#include <pork_perl.h>
#include <pork_timer.h>
#include <pork_io.h>
#include <pork_screen.h>
#include <pork_proto.h>
#include <pork_queue.h>
#include <pork_missing.h>
#include <pork_ssl.h>

struct screen globals;
static void generic_signal_handler(int sig) __noreturn;

/*
** The fallback for when no binding for a key exists.
** Insert the key into the input buffer.
*/

static void binding_insert(int key) {
	struct imwindow *win = cur_window();

	if (key > 0 && key <= 0xff)
		input_insert(win->input, key);
}

static inline void binding_run(struct pork_acct *acct, struct binding *binding)
{
	/*
	** Yeah, this is kind of a hack, but it makes things
	** more pleasant (i.e. you don't see status messages
	** about new settings and things like that if the command
	** was run from a key binding (other than hitting enter on
	** the command line).
	*/

	if (binding->key != '\n') {
		int quiet = screen_set_quiet(1);
		run_command(acct, binding->binding);
		screen_set_quiet(quiet);
	} else
		run_command(acct, binding->binding);
}

static void resize_display(void) {
	struct winsize size;

	if (ioctl(1, TIOCGWINSZ, &size) != 0) {
		debug("ioctl: %s", strerror(errno));
		pork_exit(-1, NULL, _("Fatal error getting screen size\n"));
	}

	screen_resize(size.ws_row, size.ws_col);
	screen_refresh();
}

static void sigwinch_handler(int sig __notused) {
	pork_io_add_flag(&globals, IO_ATTR_ALWAYS);
}

static void generic_signal_handler(int sig) {
	pork_exit(sig, NULL, _("Caught signal %d. Exiting\n"), sig);
}

void keyboard_input(int fd __notused,
					u_int32_t flags,
					void *data __notused)
{
	struct imwindow *win = cur_window();
	struct pork_acct *acct = win->owner;
	int key;

	/*
	** This will be the case only after the program receives SIGWINCH.
	** The screen can't be resized from inside a signal handler..
	*/
	if (flags & IO_ATTR_ALWAYS) {
		pork_io_del_flag(&globals, IO_ATTR_ALWAYS);
		resize_display();
		return;
	}

	key = wgetinput(globals.status_bar);
	if (key == -1)
		return;

	time(&acct->last_input);

	if (acct->connected && acct->marked_idle &&
		opt_get_bool(acct->prefs, ACCT_OPT_REPORT_IDLE))
	{
		if (acct->proto->set_idle_time != NULL)
			acct->proto->set_idle_time(acct, 0);
		acct->marked_idle = 0;
		screen_win_msg(cur_window(), 1, 1, 0,
			MSG_TYPE_UNIDLE, _("%s is no longer marked idle"), acct->username);
	}

	bind_exec(acct, win->active_binds, key);
}

int main(int argc, char **argv) {
	struct imwindow *win;
	time_t timer_last_run;
	time_t status_last_update = 0;
	int ret;

	if (get_options(argc, argv) != 0) {
		fprintf(stderr, _("Fatal: Error getting options.\n"));
		exit(-1);
	}

	if (initialize_environment() != 0) {
		fprintf(stderr, _("Fatal: Error initializing the terminal.\n"));
		exit(-1);
	}

	proto_init();
	color_init();
	pork_io_init();
	perl_init();

	if (screen_init(LINES, COLS) == -1)
		pork_exit(-1, NULL, _("Fatal: Error initializing the terminal.\n"));
	ssl_init();

	signal(SIGWINCH, sigwinch_handler);
	signal(SIGTERM, generic_signal_handler);
	signal(SIGQUIT, generic_signal_handler);
	signal(SIGHUP, generic_signal_handler);
	signal(SIGPIPE, SIG_IGN);

	wmove(globals.status_bar, STATUS_ROWS - 1, 0);
	win = cur_window();

	bind_init(&globals.binds);
	bind_set_handlers(&globals.binds.main, binding_run, binding_insert);
	bind_set_handlers(&globals.binds.blist, binding_run, NULL);

	alias_init(&globals.alias_hash);

	screen_set_quiet(1);
	ret = read_global_config();
	screen_set_quiet(0);

	if (ret != 0)
		screen_err_msg(_("Error reading the global configuration."));

	status_draw(globals.null_acct);
	screen_draw_input();
	screen_doupdate();

	time(&timer_last_run);
	while (1) {
		time_t time_now;
		int dirty = 0;

		pork_io_run();
		pork_acct_update();

		/*
		** Run the timers at most once per second.
		*/

		time(&time_now);
		if (timer_last_run < time_now) {
			timer_last_run = time_now;
			timer_run(&globals.timer_list);
			pork_acct_reconnect_all();
		}

		win = cur_window();
		dirty = imwindow_refresh(win);

		/*
		** Draw the status bar at most once per second.
		*/

		time(&time_now);
		if (status_last_update < time_now) {
			status_last_update = time_now;
			status_draw(win->owner);
			dirty++;
		}

		dirty += screen_draw_input();

		if (dirty)
			screen_doupdate();
	}
}

/*
** Sign off all acounts and exit. If a message
** is given, print it to the screen.
*/

void pork_exit(int status, char *msg, char *fmt, ...) {
	pork_acct_del_all(msg);
	screen_destroy();
	perl_destroy();
	pork_io_destroy();
	proto_destroy();

	wclear(stdscr);
	wrefresh(stdscr);
	delwin(stdscr);
	endwin();

	if (fmt != NULL) {
		va_list ap;

		va_start(ap, fmt);
		vfprintf(stderr, fmt, ap);
		va_end(ap);
	}

	exit(status);
}
