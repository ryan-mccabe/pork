/*
** pork_ssl.h
** Copyright (C) 2006 Ryan McCabe <ryan@numb.org>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License, version 2,
** as published by the Free Software Foundation.
*/

#ifndef __PORK_SSL_H

#include <config.h>

#ifdef SSL_SUPPORT

#include <openssl/ssl.h>
#include <openssl/bio.h>

#else

typedef struct {} SSL;
typedef struct {} SSL_CTX;

void SSL_free(SSL *);

#endif

SSL *ssl_connect(SSL_CTX *ctx, int sock);
ssize_t sock_read_ssl(void *in, void *buf, size_t len);
ssize_t sock_write_ssl(void *in, const void *buf, size_t len);

#else
#	warn "included multiple times"
#endif
