/*
** pork_queue.h - Generic queues
** Copyright (C) 2003-2005 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_QUEUE_H
#define __PORK_QUEUE_H

typedef struct {
	dlist_t *head;
	dlist_t *tail;
	u_int32_t entries;
	u_int32_t max;
} pork_queue_t;

pork_queue_t *queue_new(u_int32_t max_entries);
int queue_add(pork_queue_t *q, void *data);
int queue_putback_head(pork_queue_t *q, void *data);
void *queue_get(pork_queue_t *q);
void queue_destroy(pork_queue_t *q, void (*cleanup)(void *));

#endif
