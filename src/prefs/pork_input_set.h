/*
** pork_input_set.h - /input set command implementation.
** Copyright (C) 2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_INPUT_SET_H
#define __PORK_INPUT_SET_H

struct pork_input;

int input_init_prefs(struct pork_input *input);
struct pref_val *input_get_default_prefs(void);

enum {
	INPUT_OPT_AUTOSAVE = 0,
	INPUT_OPT_HISTORY_LEN,
	INPUT_OPT_PROMPT_STR,
	INPUT_NUM_OPTS
};

#define DEFAULT_INPUT_AUTOSAVE			0
#define DEFAULT_INPUT_HISTORY_LEN		400
#define DEFAULT_INPUT_PROMPT_STR		"%Mp%mork%w>%x "

#endif
