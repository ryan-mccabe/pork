/*
** pork.h - pork main include file.
** Copyright (C) 2002-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_H
#define __PORK_H

#ifndef _
	#define _(x) (x)
#endif

#ifdef HAVE___ATTRIBUTE__USED
	#define __used		__attribute__((used))
#else
	#define __used
#endif

#ifdef HAVE___ATTRIBUTE__UNUSED
	#define __notused	__attribute__((unused))
#else
	#define __notused
#endif

#ifdef HAVE___ATTRIBUTE__NORETURN
	#define __noreturn	__attribute__((noreturn))
#else
	#define __noreturn
#endif

#ifdef HAVE___ATTRIBUTE__FORMAT
	#define __format(x) __attribute__((format x ))
#else
	#define __format(x)
#endif

#ifdef ENABLE_DEBUGGING
#	define debug(format, args...) do { fprintf(stderr, "[%s:%u:%s] DEBUG: " format "\n", __FILE__, __LINE__, __FUNCTION__, ##args); } while (0)
#else
#	define debug(format, args...) do { } while (0)
#endif

void pork_exit(int status, char *msg, char *fmt, ...) __format((printf, 3, 4)) __noreturn;
void keyboard_input(int fd, u_int32_t condition, void *data);

#else
#	warning "included multiple times"
#endif
