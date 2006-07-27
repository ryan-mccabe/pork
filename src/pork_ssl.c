/*
** pork_ssl.c
** Copyright (C) 2006 Ryan McCabe <ryan@numb.org>
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
#include <ncurses.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <pork.h>
#include <pork_list.h>
#include <pork_missing.h>
#include <pork_util.h>
#include <pork_inet.h>
#include <pork_ssl.h>
#include <pork_input.h>
#include <pork_bind.h>
#include <pork_screen.h>

#ifdef SSL_SUPPORT

SSL *ssl_connect(SSL_CTX *ctx, int sock) {
	BIO *sbio;
	SSL *ssl;

	sbio = BIO_new_socket(sock, BIO_NOCLOSE);
	if (sbio == NULL) {
		debug("BIO_new_socket: %s", strerror(errno));
		return (NULL);
	}

	ssl = SSL_new(ctx);
	if (ssl == NULL) {
		debug("SSL_new: %s", strerror(errno));
		BIO_free_all(sbio);
		return (NULL);
	}

	SSL_set_bio(ssl, sbio, sbio);
	if (SSL_connect(ssl) != 1) {
		debug("SSL_connect: %s", strerror(errno));
		BIO_free_all(sbio);
		SSL_free(ssl);
		return (NULL);
	}

	return (ssl);
}

ssize_t sock_read_ssl(void *in, void *buf, size_t len) {
	SSL *ssl = (SSL *) in;

	return (SSL_read(ssl, buf, len));
}

ssize_t sock_write_ssl(void *out, const void *buf, size_t len) {
	SSL *ssl = (SSL *) out;

	return (SSL_write(ssl, buf, len));
}

int ssl_init(void) {
	SSL_CTX *ctx;

	SSL_library_init();
	OpenSSL_add_all_algorithms();

	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL) {
		debug("Error initializing SSL");
		return (-1);
	}

	globals.ssl_ctx = ctx;
	return (0);
}

#else

SSL *ssl_connect(SSL_CTX *ctx __notused, int sock __notused) {
	return (NULL);
}

ssize_t sock_write_ssl(void *out, const void *buf, size_t len) {
	return (-1);
}

ssize_t sock_read_ssl(void *out, void *buf, size_t len) {
	return (-1);
}

int ssl_init(void) {
	globals.ssl_ctx = NULL;
	return (0);
}

#endif
