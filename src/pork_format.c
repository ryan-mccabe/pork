/*
** pork_format.c - Facilities for filling format strings.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_cstr.h>
#include <pork_misc.h>
#include <pork_set.h>
#include <pork_set_global.h>
#include <pork_screen.h>
#include <pork_screen_io.h>
#include <pork_format.h>

/*
** Do all appropriate '$' substitutions in format strings, except for
** $>, which must be handled after everything else.
*/

int fill_format_string(	int type,
						char *buf,
						size_t len,
						struct pref_val *prefs,
						int (*handler)(char, char *, size_t, va_list), ...)
{
	va_list ap;
	char *format;
	size_t i = 0;

	format = opt_get_str(prefs, type);
	if (format == NULL) {
		debug("Unknown format str: %d", type);
		return (-1);
	}

	--len;

	while (*format != '\0' && i < len) {
		if (*format == FORMAT_VARIABLE) {
			char result[len + 1];
			int ret_code;

			format++;

			result[0] = '\0';

			va_start(ap, handler);
			ret_code = handler(*format, result, sizeof(result), ap);
			va_end(ap);

			if (ret_code == 0) {
				/*
				** The subsitution was successful.
				*/
				int ret;

				ret = xstrncpy(&buf[i], result, len - i);
				if (ret == -1)
					break;

				i += ret;
			} else if (ret_code == 1) {
				/*
				** We should suppress the whole string.
				*/

				*buf = '\0';
				break;
			} else {
				/*
				** Unknown variable -- treat
				** their FORMAT_VARIABLE as a literal.
				*/

				if (i > len - 2)
					break;

				buf[i++] = FORMAT_VARIABLE;
				buf[i++] = *format;
			}
		} else
			buf[i++] = *format;

		format++;
	}

	buf[i] = '\0';
	return (i);
}

void format_apply_justification(char *buf, chtype *ch, size_t len) {
	char *p = buf;
	char *left = NULL;
	char *right = NULL;
	size_t len_left;
	chtype fill_char;

	while ((p = strchr(p, '$')) != NULL) {
		if (p[1] == '>') {
			left = buf;

			*p = '\0';
			right = &p[2];
			break;
		}
		p++;
	}

	len_left = plaintext_to_cstr(ch, len, buf, NULL);
	fill_char = ch[len_left - 1];
	chtype_set(fill_char, ' ');

	/*
	** If there's right-justified text, paste it on.
	*/
	if (right != NULL) {
		chtype ch_right[len];
		size_t len_right;
		size_t diff;
		size_t i;
		size_t j;

		plaintext_to_cstr(ch_right, len, right, NULL);
		len_right = cstrlen(ch_right);

		/*
		** If the right side won't fit, paste as much
		** as possible, leaving at least one cell of whitespace.
		*/

		if (len_left + len_right >= len - 1) {
			int new_rlen;

			new_rlen = len_right - ((len_left + 1 + len_right) - (len - 1));
			if (new_rlen < 0)
				len_right = 0;
			else
				len_right = new_rlen;
		}

		diff = len_left + (--len - (len_left + len_right));

		i = len_left;
		while (i < diff)
			ch[i++] = fill_char;

		j = 0;
		while (i < len && j < len_right && ch_right[j] != 0)
			ch[i++] = ch_right[j++];

		ch[i] = 0;
	} else if (len_left < len - 1) {
		/*
		** If there's no right-justified text, pad out the
		** string to its maximum length with blank characters (spaces)
		** having the color/highlighting attributes of the last character
		** in the string.
		*/
		size_t i = len_left;

		--len;
		while (i < len)
			ch[i++] = fill_char;

		ch[i] = 0;
	}
}
