/*
** pork_inet.h
** Copyright (C) 2003-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_INET_H
#define __PORK_INET_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef INET6_ADDRSTRLEN
#	define MAX_IPLEN	INET6_ADDRSTRLEN
#elif defined INET_ADDRSTRLEN
#	define MAX_IPLEN	INET_ADDRSTRLEN
#else
#	define MAX_IPLEN	46
#endif

#define MAX_HOSTLEN		256

#ifndef HAVE_STRUCT_SOCKADDR_STORAGE
struct sockaddr_storage {
	struct	sockaddr __ss_sockaddr;
	char	__ss_pad[128 - sizeof(struct sockaddr)];
};
#	define ss_family __ss_sockaddr.sa_family
#endif

#ifndef HAVE_STRUCT_IN6_ADDR
struct in6_addr {
	u_int8_t s6_addr[16];
};
#endif

#ifndef HAVE_STRUCT_SOCKADDR_IN6
struct sockaddr_in6 {
	unsigned short sin6_family;
	u_int16_t sin6_port;
	u_int32_t sin6_flowinfo;
	struct in6_addr sin6_addr;
};
#endif

#ifdef HAVE___SS_FAMILY
	#define ss_family __ss_family
#endif

#define SIN4(x) ((struct sockaddr_in *) (x))
#define SIN6(x) ((struct sockaddr_in6 *) (x))
#define VALID_PORT(x) ((((x) & 0xffff) == (x)) && ((x) != 0))

int nb_connect(	struct sockaddr_storage *ss,
				struct sockaddr_storage *local,
				in_port_t port,
				int *dsock);

ssize_t sock_read_clear(void *sock, void *buf, size_t len);
ssize_t sock_write_clear(void *sock, const void *buf, size_t len);

ssize_t sock_write(	void *out,
					const void *buf,
					size_t len,
					ssize_t (*writefn)(void *, const void *, size_t));

ssize_t sock_read(	void *in,
					void *buf,
					size_t len,
					ssize_t (*writefn)(void *, void *, size_t));

int sockprintf(int fd, const char *fmt, ...) __format((printf, 2, 3));
int get_port(const char *name, in_port_t *port);
int get_addr(const char *hostname, struct sockaddr_storage *addr);
int get_peer_addr(int sock, struct sockaddr_storage *ss);
int get_local_addr(int sock, struct sockaddr_storage *ss);
void *sin_addr(struct sockaddr_storage *ss);
size_t sin_len(const struct sockaddr_storage *ss);
in_port_t sin_port(const struct sockaddr_storage *ss);
void sin_set_port(struct sockaddr_storage *ss, in_port_t port);
int sock_setflags(int sock, u_int32_t flags);
int sock_is_error(int sock);
int sock_listen(struct sockaddr_storage *ss, in_port_t port);

int get_hostname(struct sockaddr_storage *addr,
						char *hostbuf,
						size_t len);

void get_ip(struct sockaddr_storage *ss,
			char *buf,
			size_t len);

#else
#	warning "included multiple times"
#endif
