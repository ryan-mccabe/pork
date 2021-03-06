/*
** pork_input.c - line editing and history
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** I've tried to keep the input handler code independent of everything else
** so that it might be useful outside this particular program.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_list.h>
#include <pork_set.h>
#include <pork_input_set.h>
#include <pork_cstr.h>
#include <pork_command.h>
#include <pork_input.h>

static inline void input_free(void *param __notused, void *data);

/*
** Deletes the character at the cursor position.
*/

inline void input_delete(struct pork_input *input) {
	u_int32_t cur = input->cur - input->prompt_len;

	if (input->input_buf[cur] == '\0')
		return;

	memmove(&input->input_buf[cur], &input->input_buf[cur + 1],
		input->len - cur);

	input->begin_completion = input->cur;
	input->len--;
	input->dirty = 1;
}

/*
** Deletes the character before the cursor.
*/

inline void input_bkspace(struct pork_input *input) {
	u_int32_t cur = input->cur - input->prompt_len;

	if (cur == 0)
		return;

	cur--;

	memmove(&input->input_buf[cur], &input->input_buf[cur + 1],
		input->len - cur);

	input->cur -= 1;
	input->begin_completion = input->cur;
	input->len--;
	input->dirty = 1;
}

/*
** Add the character @c at the cursor position.
*/

inline void input_insert(struct pork_input *input, int c) {
	u_int32_t cur = input->cur - input->prompt_len;

	if (input->len >= sizeof(input->input_buf) - 1) {
		beep();
		return;
	}

	memmove(&input->input_buf[cur + 1], &input->input_buf[cur],
		input->len - cur + 1);

	input->input_buf[cur] = c;
	input->cur++;
	input->begin_completion = input->cur;
	input->len++;
	input->dirty = 1;
}

/*
** Insert the string @str at the cursor position.
*/

inline void input_insert_str(struct pork_input *input, char *str) {
	u_int32_t cur = input->cur - input->prompt_len;
	size_t len = strlen(str);

	if (input->len + len >= sizeof(input->input_buf))
		return;

	memmove(&input->input_buf[cur + len], &input->input_buf[cur],
		input->len - cur + 1);

	memcpy(&input->input_buf[cur], str, len);
	input->cur += len;
	input->begin_completion = input->cur;
	input->len += len;
	input->dirty = 1;
}

/*
** Clear the input line.
*/

inline void input_clear_line(struct pork_input *input) {
	input->input_buf[0] = '\0';
	input->cur = input->prompt_len;
	input->begin_completion = input->cur;
	input->len = 0;
	input->dirty = 1;
}

/*
** Clear from the cursor position the start of the line.
*/

inline void input_clear_to_start(struct pork_input *input) {
	u_int32_t cur = input->cur - input->prompt_len;
	u_int32_t new_len = input->len - cur;

	memmove(&input->input_buf[0], &input->input_buf[cur], new_len + 1);
	input->len = new_len;
	input->cur = input->prompt_len;
	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Clear from the cursor position to the end of the line.
*/

inline void input_clear_to_end(struct pork_input *input) {
	u_int32_t cur = input->cur - input->prompt_len;

	input->len = cur;
	input->input_buf[cur] = '\0';
	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Deletes @num characters starting at the current cursor
** position. If @num is negative, it deletes from the cursor
** back, otherwise it deletes from the cursor forward.
*/

inline void input_remove(struct pork_input *input, int num) {
	u_int32_t cur = input->cur - input->prompt_len;

	if (num < 0) {
		num = -num;
		if ((u_int32_t) num >= cur) {
			input_clear_to_start(input);
			return;
		}

		memmove(&input->input_buf[cur - num], &input->input_buf[cur],
			input->len - cur + 1);

		input->cur -= num;
		input->len -= num;
	} else {
		if (cur + num >= input->len) {
			input_clear_to_end(input);
			return;
		}

		input->len -= num;
		memmove(&input->input_buf[cur], &input->input_buf[cur + num],
			input->len - cur + 1);
	}

	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Move to the start of the line.
*/

inline void input_home(struct pork_input *input) {
	input->cur = input->prompt_len;
	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Move to the end of the line.
*/

inline void input_end(struct pork_input *input) {
	input->cur = input->len + input->prompt_len;
	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Move the cursor left.
*/

inline void input_move_left(struct pork_input *input) {
	if (input->cur > input->prompt_len) {
		input->cur--;
		input->dirty = 1;
	}

	input->begin_completion = input->cur;
}

/*
** Move the cursor right.
*/

inline void input_move_right(struct pork_input *input) {
	if (input->cur < input->len + input->prompt_len) {
		input->cur++;
		input->dirty = 1;
	}

	input->begin_completion = input->cur;
}

/*
** Move the cursor to the start of the previous
** word.
*/

void input_prev_word(struct pork_input *input) {
	u_int32_t cur = input->cur - input->prompt_len;

	if (cur == 0)
		return;

	while (cur > 0 && input->input_buf[--cur] == ' ')
		;

	if (input->input_buf[cur] == ' ')
		goto start_of_line;

	while (input->input_buf[cur] != ' ') {
		if (cur == 0)
			goto start_of_line;
		cur--;
	}

	input->cur = cur + 1 + input->prompt_len;
	input->begin_completion = input->cur;
	input->dirty = 1;
	return;

start_of_line:
	input->cur = input->prompt_len;
	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Clear from the cursor position to the start of the
** previous word.
*/

void input_clear_prev_word(struct pork_input *input) {
	u_int32_t cur;
	u_int32_t del_end = input->cur - input->prompt_len;
	u_int32_t old_end = input->len;

	if (input->cur == input->prompt_len)
		return;

	input_prev_word(input);

	cur = input->cur - input->prompt_len;
	memmove(&input->input_buf[cur], &input->input_buf[del_end],
		old_end - del_end + 1);

	input->len -= del_end - cur;
	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Move the cursor to the start of the next word.
*/

void input_next_word(struct pork_input *input) {
	u_int32_t cur = input->cur - input->prompt_len;

	if (cur == input->len)
		return;

	while (cur < input->len && input->input_buf[cur] == ' ')
		cur++;

	while (cur < input->len && input->input_buf[cur] != ' ')
		cur++;

	input->cur = cur + input->prompt_len;

	if (input->input_buf[cur] == ' ')
		input->cur++;

	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Clear the next word after the cursor.
*/

void input_clear_next_word(struct pork_input *input) {
	u_int32_t cur;
	u_int32_t del_start = input->cur - input->prompt_len;
	u_int32_t old_end = input->len;

	if (input->cur == input->len + input->prompt_len)
		return;

	input_next_word(input);

	cur = input->cur - input->prompt_len;
	memmove(&input->input_buf[del_start], &input->input_buf[cur],
		old_end - cur + 1);

	input->len -= cur - del_start;
	input->cur = del_start + input->prompt_len;
	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Trim the command history list so that it doesn't
** exceed the maximum length that's been set for it.
*/

void input_history_prune(struct pork_input *input) {
	dlist_t *cur = input->history_end;
	u_int32_t history_max = opt_get_int(input->prefs, INPUT_OPT_HISTORY_LEN);

	while (input->history_len > history_max && cur != NULL) {
		dlist_t *prev = cur->prev;

		free(cur->data);
		dlist_remove(input->history, cur);
		input->history_len--;
		cur = prev;
	}

	input->history_end = cur;
}

/*
** Add a line to the history list.
*/

void input_history_add(struct pork_input *input) {
	u_int32_t history_max = opt_get_int(input->prefs, INPUT_OPT_HISTORY_LEN);

	input->history = dlist_add_head(input->history, xstrdup(input->input_buf));
	input->history_cur = NULL;
	input->history_len++;

	if (input->history_end == NULL)
		input->history_end = input->history;

	if (input->history_len > history_max)
		input_history_prune(input);
}

/*
** Extract a line from the history list, and
** copy it to the input line.
*/

void input_history_prev(struct pork_input *input) {
	if (input->history == NULL)
		return;

	if (input->history_cur == NULL) {
		dlist_t *cur = input->history;

		if (input->len != 0)
			input_history_add(input);

		input->history_cur = cur;
	} else if (input->history_cur->next != NULL)
		input->history_cur = input->history_cur->next;
	else
		return;

	xstrncpy(input->input_buf, (char *) input->history_cur->data,
		sizeof(input->input_buf));

	input->cur = strlen(input->input_buf) + input->prompt_len;
	input->len = input->cur - input->prompt_len;
	input->begin_completion = input->cur;
	input->dirty = 1;
}

/*
** Select the next (a more recent) line from the
** history buffer, and write it to the input line.
*/

void input_history_next(struct pork_input *input) {
	if (input->history_cur == NULL)
		return;

	input->dirty = 1;

	/*
	** If the user hits down when they're already at the end
	** of the history list, clear the line.
	*/

	if (input->history_cur->prev == NULL) {
		input->history_cur = NULL;
		input->input_buf[0] = '\0';
		input->cur = input->prompt_len;
		input->begin_completion = input->cur;
		input->len = 0;

		return;
	}

	input->history_cur = input->history_cur->prev;

	xstrncpy(input->input_buf, (char *) input->history_cur->data,
		sizeof(input->input_buf));

	input->cur = strlen(input->input_buf) + input->prompt_len;
	input->len = input->cur - input->prompt_len;
	input->begin_completion = input->cur;
}

/*
** Clear the input history list.
*/

inline void input_history_clear(struct pork_input *input) {
	dlist_destroy(input->history, NULL, input_free);
	input->history = NULL;
	input->history_end = NULL;
	input->history_cur = NULL;
	input->history_len = 0;

	input->input_buf[0] = '\0';
	input->cur = input->prompt_len;
	input->begin_completion = input->cur;
	input->len = 0;

	input->dirty = 1;
}

/*
** Initialize the input handler.
*/

void input_init(struct pork_input *input, u_int32_t width) {
	char *prompt;

	memset(input, 0, sizeof(*input));
	input->width = width;
	input->dirty = 1;
	input_init_prefs(input);

	prompt = opt_get_str(input->prefs, INPUT_OPT_PROMPT_STR);
	if (prompt != NULL)
		input_set_prompt(input, prompt);
}

static inline void input_free(void *param __notused, void *data) {
	free(data);
}

inline void input_destroy(struct pork_input *input) {
	free(input->prompt);
	dlist_destroy(input->history, NULL, input_free);
}

inline void input_resize(struct pork_input *input, u_int32_t width) {
	input->width = width;
	input->dirty = 1;
}

char *input_partial(struct pork_input *input) {
	u_int32_t offset;

	if (input->prompt_len >= input->width)
		return (NULL);

	if (input->cur >= input->width)
		offset = (input->cur / input->width) * input->width - input->prompt_len;
	else
		offset = 0;

	return (&input->input_buf[offset]);
}

/*
** Returns the x position of the cursor.
*/

inline u_int32_t input_get_cursor_pos(struct pork_input *input) {
	return (input->cur % input->width);
}

/*
** Set the input prompt to the string specified by @prompt
*/

int input_set_prompt(struct pork_input *input, char *prompt) {
	u_int32_t cur;

	if (prompt != NULL && strlen(prompt) >= input->width)
		return (-1);

	cur = input->cur - input->prompt_len;
	free(input->prompt);

	if (prompt == NULL) {
		input->prompt = NULL;
		input->prompt_len = 0;
	} else {
		size_t tmp_len = strlen(prompt) + 1;

		input->prompt = xmalloc(sizeof(chtype) * tmp_len);
		plaintext_to_cstr(input->prompt, tmp_len, prompt, NULL);
		input->prompt_len = cstrlen(input->prompt);
	}

	input->cur = cur + input->prompt_len;
	input->begin_completion = input->cur;
	input->dirty = 1;

	return (0);
}

/*
** Fetches the contents of the input buffer as a C string.
*/

inline char *input_get_buf_str(struct pork_input *input) {
	return (input->input_buf);
}

/*
** Sets the input buffer to the specified string.
*/

int input_set_buf(struct pork_input *input, char *str) {
	size_t len = strlen(str);

	if (str == NULL || len >= sizeof(input->input_buf) - 1)
		return (-1);

	if (xstrncpy(input->input_buf, str, sizeof(input->input_buf)) == -1) {
		input->input_buf[0] = '\0';
		input->len = 0;
		input->cur = 0;
		return (-1);
	}

	input->len = len;
	input->cur = len;
	input->begin_completion = input->cur;

	return (0);
}

/*
** Send the input string provided, or the current
** input buffer, if no string is provided.
*/

int input_send(struct pork_acct *acct, struct pork_input *input, char *args) {
	static int recursion;

	/*
	** This is kind of a hack, but it's necessary if the client
	** isn't to crash when someone types '/input send'
	*/

	if (recursion == 1 && args == NULL) {
		return (-1);
	}

	recursion = 1;

	if (args != NULL)
		input_set_buf(input, args);

	if (input->len > 0) {
		char *input_str = xstrdup(input_get_buf_str(input));

		if (args == NULL)
			input_history_add(input);

		input_clear_line(input);
		command_enter_str(acct, input_str);
		free(input_str);
	}

	recursion = 0;
	return (0);
}
