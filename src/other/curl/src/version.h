#ifndef __VERSION_H
#define __VERSION_H
/***************************************************************************
 *                                  _   _ ____  _     
 *  Project                     ___| | | |  _ \| |    
 *                             / __| | | | |_) | |    
 *                            | (__| |_| |  _ <| |___ 
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2008, Daniel Stenberg, <daniel@haxx.se>, et al.
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
 * $Id: version.h,v 1.94 2008-01-24 14:15:49 gknauf Exp $
 ***************************************************************************/

#include <curl/curlver.h>

#define CURL_NAME "curl"
#define CURL_COPYRIGHT LIBCURL_COPYRIGHT
#define CURL_VERSION "7.19.4"
#define CURL_VERSION_MAJOR LIBCURL_VERSION_MAJOR
#define CURL_VERSION_MINOR LIBCURL_VERSION_MINOR
#define CURL_VERSION_PATCH LIBCURL_VERSION_PATCH
#define CURL_ID CURL_NAME " " CURL_VERSION " (" OS ") "

#endif
