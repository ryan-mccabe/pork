/*
** pork_missing.h
** Copyright (C) 2003-2004 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_MISSING_H
#define __PORK_MISSING_H

#ifndef HAVE_STRSEP
char *strsep(char **str, char *delim);
#endif

#ifndef HAVE_VSNPRINTF
int vsnprintf(char *str, size_t n, char *fmt, va_list ap);
#endif

#ifndef HAVE_SNPRINTF
int snprintf(char *str, size_t n, char const *fmt, ...);
#endif

#ifndef HAVE_INET_NTOP
const char *inet_ntop(int af, const void *src, char *dst, size_t len);
#endif

#ifndef EAI_NODATA
#	define EAI_NODATA	1
#	define EAI_MEMORY	2
#endif

#ifndef AI_PASSIVE
#	define AI_PASSIVE		1
#	define AI_CANONNAME		2
#endif

#ifndef NI_NUMERICHOST
#	define NI_NUMERICHOST	2
#	define NI_NAMEREQD		4
#	define NI_NUMERICSERV	8
#endif

#if !defined HAVE_STRUCT_ADDRINFO || !defined HAVE_GETNAMEINFO
struct sockaddr;
#endif


#ifndef HAVE_STRUCT_ADDRINFO

struct addrinfo {
	int	ai_flags;
	int	ai_family;
	int	ai_socktype;
	int	ai_protocol;
	size_t ai_addrlen;
	char *ai_canonname;
	struct sockaddr *ai_addr;
	struct addrinfo *ai_next;
};

#endif

#ifndef HAVE_GETADDRINFO
int getaddrinfo(const char *hostname, const char *servname,
				const struct addrinfo *hints, struct addrinfo **res);
#endif

#ifndef HAVE_GAI_STRERROR
char *gai_strerror(int ecode);
#endif

#ifndef HAVE_FREEADDRINFO
void freeaddrinfo(struct addrinfo *ai);
#endif

#ifndef HAVE_GETNAMEINFO
int getnameinfo(const struct sockaddr *sa, size_t salen, char *host,
				size_t hostlen, char *serv, size_t servlen, int flags);
#endif

#ifndef HAVE_VASPRINTF
int vasprintf(char **result, const char *format, va_list args);
#endif

#ifndef NI_MAXSERV
#	define NI_MAXSERV 32
#endif

#ifndef NI_MAXHOST
#	define NI_MAXHOST 1025
#endif

#endif
