/*
** pork_io.h - I/O handler.
** Copyright (C) 2003-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_IO_H
#define __PORK_IO_H

#define IO_COND_READ		0x01
#define IO_COND_WRITE		0x02
#define IO_COND_EXCEPTION	0x04
#define IO_COND_RW			(IO_COND_READ | IO_COND_WRITE)
#define IO_COND_ANY			(IO_COND_READ | IO_COND_WRITE | IO_COND_EXCEPTION)
#define IO_COND(flags)		((flags) & 0x0000ffff)

#define IO_ATTR_SSL			0x10000
#define IO_ATTR_DEAD		0x20000
#define IO_ATTR_ALWAYS		0x40000
#define IO_ATTR(flags)		((flags) & 0xffff0000)

struct io_source {
	int fd;
	u_int32_t flags;
	void *data;
	void *key;
	void (*callback)(int fd, u_int32_t flags, void *data);
};

int pork_io_init(void);
void pork_io_destroy(void);
int pork_io_del(void *key);
int pork_io_run(void);
int pork_io_dead(void *key);
int pork_io_add_flag(void *key, u_int32_t new_flag);
int pork_io_del_flag(void *key, u_int32_t new_flag);
int pork_io_set_flags(void *key, u_int32_t new_flags);

int pork_io_add(int fd,
				u_int32_t flags,
				void *data,
				void *key,
				void (*callback)(int fd, u_int32_t flags, void *data));

#else
#	warning "included multiple times"
#endif
