/*
** pork_input.h - line editing and history
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_INPUT_H
#define __PORK_INPUT_H

#define INPUT_BUFFER_LEN	4096
#define MAX_HISTORY_LEN		150

#define META_MASK		(0xf << 28)
#define META_NUM(x)		(((x) & META_MASK) >> 28)
#define META_KEY(x, n)	((x) | ((n) << 28))

#define CTRL_KEY(x)		((x) & 0x1f)

struct pref_val;
struct pork_acct;

struct pork_input {
	u_int32_t width;
	u_int16_t cur;
	u_int16_t len;
	u_int16_t begin_completion;
	u_int16_t history_len;
	u_int16_t prompt_len;
	u_int16_t dirty:1;
	chtype *prompt;
	dlist_t *history;
	dlist_t *history_cur;
	dlist_t *history_end;
	char input_buf[INPUT_BUFFER_LEN];
	struct pref_val *prefs;
};

char *input_partial(struct pork_input *input);
void input_resize(struct pork_input *input, u_int32_t width);
void input_destroy(struct pork_input *input);
void input_init(struct pork_input *input, u_int32_t width);

void input_delete(struct pork_input *input);
void input_bkspace(struct pork_input *input);
void input_remove(struct pork_input *input, int num);
void input_insert(struct pork_input *input, int c);
void input_insert_str(struct pork_input *input, char *str);
void input_clear_line(struct pork_input *input);
void input_clear_to_end(struct pork_input *input);
void input_clear_to_start(struct pork_input *input);
void input_home(struct pork_input *input);
void input_end(struct pork_input *input);
void input_prev_word(struct pork_input *input);
void input_next_word(struct pork_input *input);
void input_clear_prev_word(struct pork_input *input);
void input_clear_next_word(struct pork_input *input);
void input_move_left(struct pork_input *input);
void input_move_right(struct pork_input *input);
void input_history_prune(struct pork_input *input);
void input_history_add(struct pork_input *input);
void input_history_next(struct pork_input *input);
void input_history_prev(struct pork_input *input);
void input_history_clear(struct pork_input *input);
int input_set_buf(struct pork_input *input, char *str);
int input_set_prompt(struct pork_input *input, char *prompt);
int input_send(struct pork_acct *acct, struct pork_input *input, char *args);
char *input_get_buf_str(struct pork_input *input);
u_int32_t input_get_cursor_pos(struct pork_input *input);

#else
#	warning "included multiple times"
#endif
