/*
** pork_inet.c
** Copyright (C) 2003-2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#include <config.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>

#include <pork.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_inet.h>

ssize_t sock_read_clear(void *sock, void *buf, size_t len) {
	return (read(*(int *) sock, buf, len));
}

ssize_t sock_write_clear(void *sock, const void *buf, size_t len) {
	return (write(*(int *) sock, buf, len));
}

/*
** Write to a socket, deal with interrupted and incomplete writes. Returns
** the number of characters written to the socket on success, -1 on failure.
*/

ssize_t sock_write(	void *out,
					const void *buf,
					size_t len,
					ssize_t (*writefn)(void *, const void *, size_t))
{
	ssize_t written = 0;

	while (len > 0) {
		ssize_t n;

		n = writefn(out, buf, len);
		if (n < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;

			debug("sock: %s", strerror(errno));
			return (-1);
		}

		written += n;
		len -= n;
		buf = (const char *) buf + n;
	}

	return (written);
}

/*
** Returns -1 if the connection died, 0 otherwise.
*/

ssize_t sock_read(	void *in,
					void *buf,
					size_t len,
					ssize_t (*readfn)(void *, void *, size_t))
{
	int i;
	ssize_t ret = 0;

	for (i = 0 ; i < 5 ; i++) {
		ret = readfn(in, buf, len - 1);
		if (ret < 0) {
			if (errno == EINTR || errno == EAGAIN)
				continue;

			debug("read: %s", strerror(errno));
			return (-1);
		}

		if (ret == 0) {
			debug("read: %s", strerror(errno));
			return (-1);
		}

		*((char *) buf + ret) = '\0';
		return (ret);
	}

	return (-1);
}

/*
** printf-like function that writes to sockets.
*/

#ifdef HAVE_VASPRINTF

int sockprintf(int fd, const char *fmt, ...) {
	va_list ap;
	char *buf;
	ssize_t ret;

	va_start(ap, fmt);
	ret = vasprintf(&buf, fmt, ap);
	va_end(ap);

	if (ret < 0)
		return (-1);

	ret = sock_write(&fd, buf, ret, sock_write_clear);
	free(buf);

	return (ret);
}

#else

int sockprintf(int fd, const char *fmt, ...) {
	va_list ap;
	char buf[4096];
	int ret;

	va_start(ap, fmt);
	ret = vsnprintf(buf, sizeof(buf), fmt, ap);
	if (ret < 0 || (size_t) ret >= sizeof(buf))
		ret = -1;
	va_end(ap);

	ret = sock_write(&fd, buf, strlen(buf), sock_write_clear);
	return (ret);
}

#endif

inline int get_peer_addr(int sock, struct sockaddr_storage *ss) {
	socklen_t len = sizeof(*ss);
	int ret;

	ret = getpeername(sock, (struct sockaddr *) ss, &len);
	if (ret != 0)
		debug("getpeername: %s", strerror(errno));

	return (ret);
}

inline int get_local_addr(int sock, struct sockaddr_storage *ss) {
	socklen_t len = sizeof(*ss);
	int ret;

	ret = getsockname(sock, (struct sockaddr *) ss, &len);
	if (ret != 0)
		debug("getsockname: %s", strerror(errno));

	return (ret);
}

/*
** Return string IPv4 or IPv6 address.
*/

inline void get_ip(	struct sockaddr_storage *ss,
					char *buf,
					size_t len)
{
	if (inet_ntop(ss->ss_family, sin_addr(ss), buf, len) == NULL)
		debug("inet_ntop: %s", strerror(errno));
}

/*
** Returns the address set in the appropriate
** sockaddr struct.
*/

void *sin_addr(struct sockaddr_storage *ss) {
	if (ss->ss_family == AF_INET6)
		return (&SIN6(ss)->sin6_addr);

	return (&SIN4(ss)->sin_addr);
}

/*
** Returns the length of the sockaddr struct.
*/

inline size_t sin_len(const struct sockaddr_storage *ss) {
	if (ss->ss_family == AF_INET6)
		return (sizeof(struct sockaddr_in6));

	return (sizeof(struct sockaddr_in));
}

/*
** Returns the port set in the sockaddr struct.
*/

inline in_port_t sin_port(const struct sockaddr_storage *ss) {
	if (ss->ss_family == AF_INET6)
		return (SIN6(ss)->sin6_port);

	return (SIN4(ss)->sin_port);
}

/*
** Sets the port for the approprite socket family.
*/

void sin_set_port(struct sockaddr_storage *ss, in_port_t port) {
	if (ss->ss_family == AF_INET6)
		SIN6(ss)->sin6_port = port;

	SIN4(ss)->sin_port = port;
}

/*
** Return the canonical hostname of the given address.
*/

inline int get_hostname(struct sockaddr_storage *addr,
						char *hostbuf,
						size_t len)
{
	int ret;

	ret = getnameinfo((struct sockaddr *) addr, sizeof(struct sockaddr_storage),
					hostbuf, len, NULL, 0, 0);

	if (ret != 0)
		debug("getnameinfo: %s", strerror(errno));

	return (ret);
}

/*
** Get the port associated with a tcp service name.
*/

int get_port(const char *name, in_port_t *port) {
	struct servent *servent;

	servent = getservbyname(name, "tcp");
	if (servent != NULL)
		*port = ntohs(servent->s_port);
	else {
		char *end;
		long temp_port;

		temp_port = strtol(name, &end, 10);

		if (*end != '\0') {
			debug("invalid port: %s", name);
			return (-1);
		}

		if (!VALID_PORT(temp_port)) {
			debug("invalid port: %s", name);
			return (-1);
		}

		*port = temp_port;
	}

	return (0);
}

/*
** Return a network byte ordered ipv4 or ipv6 address.
*/

int get_addr(const char *hostname, struct sockaddr_storage *addr) {
	struct addrinfo *res = NULL;
	size_t len;
	int ret;

	if ((ret = getaddrinfo(hostname, NULL, NULL, &res)) != 0) {
		if (res != NULL)
			freeaddrinfo(res);

		debug("getaddrinfo: %s: %s", hostname, gai_strerror(ret));
		return (-1);
	}

	switch (res->ai_addr->sa_family) {
		case AF_INET:
			len = sizeof(struct sockaddr_in);
			break;

		case AF_INET6:
			len = sizeof(struct sockaddr_in6);
			break;

		default:
			debug("unknown protocol family: %d", res->ai_addr->sa_family);
			goto out_fail;
	}

	if (len < res->ai_addrlen) {
		debug("%u < %u", len, res->ai_addrlen);
		goto out_fail;
	}

	memcpy(addr, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);

	return (0);

out_fail:
	freeaddrinfo(res);
	return (-1);
}

inline int sock_setflags(int sock, u_int32_t flags) {
	int ret;

	ret = fcntl(sock, F_SETFL, flags);
	if (ret == -1)
		debug("fnctl: %s", strerror(errno));

	return (ret);
}

int nb_connect(	struct sockaddr_storage *dest,
				struct sockaddr_storage *laddr,
				in_port_t dport,
				int *dsock)
{
	int sock;
	int ret = 0;

	sock = socket(dest->ss_family, SOCK_STREAM, 0);
	if (sock < 0) {
		debug("socket: %s", strerror(errno));
		return (-1);
	}

	if (laddr != NULL) {
		if (bind(sock, (struct sockaddr *) laddr, sin_len(laddr)) != 0) {
			debug("bind: %s", strerror(errno));
			goto err_out;
		}
	}

	if (sock_setflags(sock, O_NONBLOCK) == -1)
		goto err_out;

	sin_set_port(dest, htons(dport));

	if (connect(sock, (struct sockaddr *) dest, sin_len(dest)) != 0) {
		if (errno != EINPROGRESS) {
			debug("connect: %s", strerror(errno));
			goto err_out;
		}

		ret = -EINPROGRESS;
	} else {
		if (sock_setflags(sock, 0) == -1)
			goto err_out;
	}

	if (ret == 0)
		debug("immediate connect");

	*dsock = sock;
	return (ret);

err_out:
	close(sock);
	return (-1);
}

int sock_is_error(int sock) {
	int error;
	socklen_t errlen = sizeof(error);
	int ret;

	ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &errlen);
	if (ret == -1) {
		if (errno == ENOTSOCK)
			return (0);

		debug("getsockopt: %d: %s", sock, strerror(errno));
		return (errno);
	}

	return (error);
}

/*
** Listen to the specified port (passed in host byte order) on the address
** specified by ss.
*/

int sock_listen(struct sockaddr_storage *ss, in_port_t listen_port) {
	int sock;
	struct addrinfo *cur;
	const int one = 1;

	if (ss == NULL)
		return (-1);

	cur = xcalloc(1, sizeof(*cur));
	cur->ai_family = ss->ss_family;

	switch (cur->ai_family) {
		case AF_INET6:
			cur->ai_addrlen = sizeof(struct sockaddr_in6);
			break;

		case AF_INET:
			cur->ai_addrlen = sizeof(struct sockaddr_in);
			break;

		default:
			debug("unknown protocol family: %d", cur->ai_family);
			free(cur);
			return (-1);
	}

	cur->ai_addr = xmalloc(cur->ai_addrlen);
	memcpy(cur->ai_addr, ss, cur->ai_addrlen);

	if (cur->ai_family == AF_INET)
		SIN4(cur->ai_addr)->sin_port = htons(listen_port);
	else
		SIN6(cur->ai_addr)->sin6_port = htons(listen_port);

	sock = socket(cur->ai_family, SOCK_STREAM, 0);
	if (sock == -1) {
		debug("socket: %s", strerror(errno));
		goto done;
	}

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) != 0) {
		debug("setsockopt: %s", strerror(errno));
		goto err_out;
	}

	if (bind(sock, cur->ai_addr, cur->ai_addrlen) != 0) {
		debug("bind: %s", strerror(errno));
		goto err_out;
	}

	if (listen(sock, SOMAXCONN) != 0) {
		debug("listen: %s", strerror(errno));
		goto err_out;
	}

	if (sock_setflags(sock, O_NONBLOCK) == -1)
		goto err_out;

done:
	free(cur->ai_addr);
	free(cur);
	return (sock);

err_out:
	free(cur->ai_addr);
	free(cur);
	close(sock);
	return (-1);
}
