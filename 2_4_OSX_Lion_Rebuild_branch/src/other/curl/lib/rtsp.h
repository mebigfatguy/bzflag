#ifndef __RTSP_H_
#define __RTSP_H_

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
#ifndef CURL_DISABLE_RTSP

extern const struct Curl_handler Curl_handler_rtsp;

/*
 * Parse and write out any available RTP data.
 *
 * nread: amount of data left after k->str. will be modified if RTP
 *        data is parsed and k->str is moved up
 * readmore: whether or not the RTP parser needs more data right away
 */
CURLcode Curl_rtsp_rtp_readwrite(struct SessionHandle *data,
                                 struct connectdata *conn,
                                 ssize_t *nread,
                                 bool *readmore);


/* protocol-specific functions set up to be called by the main engine */
CURLcode Curl_rtsp(struct connectdata *conn, bool *done);
CURLcode Curl_rtsp_done(struct connectdata *conn, CURLcode, bool premature);
CURLcode Curl_rtsp_connect(struct connectdata *conn, bool *done);
CURLcode Curl_rtsp_disconnect(struct connectdata *conn);

CURLcode Curl_rtsp_parseheader(struct connectdata *conn, char *header);

#endif /* CURL_DISABLE_RTSP */

/*
 * RTSP Connection data
 *
 * Currently, only used for tracking incomplete RTP data reads
 */
struct rtsp_conn {
  char *rtp_buf;
  ssize_t rtp_bufsize;
  int rtp_channel;
};

/****************************************************************************
 * RTSP unique setup
 ***************************************************************************/
struct RTSP {
  /*
   * http_wrapper MUST be the first element of this structure for the wrap
   * logic to work. In this way, we get a cheap polymorphism because
   * &(data->state.proto.rtsp) == &(data->state.proto.http) per the C spec
   *
   * HTTP functions can safely treat this as an HTTP struct, but RTSP aware
   * functions can also index into the later elements.
   */
  struct HTTP http_wrapper; /*wrap HTTP to do the heavy lifting */

  long CSeq_sent; /* CSeq of this request */
  long CSeq_recv; /* CSeq received */
};


#endif /* __RTSP_H_ */
