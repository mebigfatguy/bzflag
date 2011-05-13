#ifndef __SSLUSE_H
#define __SSLUSE_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2010, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

#ifdef USE_SSLEAY
/*
 * This header should only be needed to get included by sslgen.c and ssluse.c
 */

#include "urldata.h"
CURLcode Curl_ossl_connect(struct connectdata *conn, int sockindex);
CURLcode Curl_ossl_connect_nonblocking(struct connectdata *conn,
                                       int sockindex,
                                       bool *done);

/* close a SSL connection */
void Curl_ossl_close(struct connectdata *conn, int sockindex);

/* tell OpenSSL to close down all open information regarding connections (and
   thus session ID caching etc) */
int Curl_ossl_close_all(struct SessionHandle *data);

/* Sets an OpenSSL engine */
CURLcode Curl_ossl_set_engine(struct SessionHandle *data, const char *engine);

/* function provided for the generic SSL-layer, called when a session id
   should be freed */
void Curl_ossl_session_free(void *ptr);

/* Sets engine as default for all SSL operations */
CURLcode Curl_ossl_set_engine_default(struct SessionHandle *data);

/* Build list of OpenSSL engines */
struct curl_slist *Curl_ossl_engines_list(struct SessionHandle *data);

int Curl_ossl_init(void);
void Curl_ossl_cleanup(void);

/* for documentation see Curl_ssl_send() in sslgen.h */
ssize_t Curl_ossl_send(struct connectdata *conn,
                       int sockindex,
                       const void *mem,
                       size_t len,
                       int *curlcode);

/* for documentation see Curl_ssl_recv() in sslgen.h */
ssize_t Curl_ossl_recv(struct connectdata *conn, /* connection data */
                       int num,                  /* socketindex */
                       char *buf,                /* store read data here */
                       size_t buffersize,        /* max amount to read */
                       int *curlcode);

size_t Curl_ossl_version(char *buffer, size_t size);
int Curl_ossl_check_cxn(struct connectdata *cxn);
int Curl_ossl_seed(struct SessionHandle *data);

int Curl_ossl_shutdown(struct connectdata *conn, int sockindex);
bool Curl_ossl_data_pending(const struct connectdata *conn,
                            int connindex);

/* API setup for OpenSSL */
#define curlssl_init Curl_ossl_init
#define curlssl_cleanup Curl_ossl_cleanup
#define curlssl_connect Curl_ossl_connect
#define curlssl_connect_nonblocking Curl_ossl_connect_nonblocking
#define curlssl_session_free(x) Curl_ossl_session_free(x)
#define curlssl_close_all Curl_ossl_close_all
#define curlssl_close Curl_ossl_close
#define curlssl_shutdown(x,y) Curl_ossl_shutdown(x,y)
#define curlssl_set_engine(x,y) Curl_ossl_set_engine(x,y)
#define curlssl_set_engine_default(x) Curl_ossl_set_engine_default(x)
#define curlssl_engines_list(x) Curl_ossl_engines_list(x)
#define curlssl_send Curl_ossl_send
#define curlssl_recv Curl_ossl_recv
#define curlssl_version Curl_ossl_version
#define curlssl_check_cxn Curl_ossl_check_cxn
#define curlssl_data_pending(x,y) Curl_ossl_data_pending(x,y)

#endif /* USE_SSLEAY */
#endif /* __SSLUSE_H */
