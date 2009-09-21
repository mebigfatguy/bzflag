/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "network.h"

//Includes common to all platforms
#include "ErrorHandler.h"
#include "Address.h"
#include <stdio.h>
#include <string.h>
#include <vector>
#include <string>


//============================================================================//
#if !defined(WIN32)
//============================================================================//

#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#ifndef HAVE_HSTRERROR
// POSIX.1-2001 marks this "obsolete" but we need it to interpret
// errors from gethostbyname(), which is likewise obsolete
char* hstrerror(int err)
{
  switch (err) {
  case 0:
    return "Resolver Error 0 (no error)";
  case HOST_NOT_FOUND:
    return "Unknown host";
  case TRY_AGAIN:
    return "Host name lookup failure";
  case NO_RECOVERY:
    return "Unknown server error";
  case NO_DATA:
    return "No address associated with name";
  default:
    return "Unknown resolver error";
    }
}
#endif


#ifndef O_NDELAY
#  define O_NDELAY O_NONBLOCK
#endif


extern "C" {

const char* socket_strerror(int err)
{
  return strerror(err);
}


void nerror(const char* msg)
{
  std::vector<std::string> args;
  if (msg) {
    args.push_back(msg);
    args.push_back(strerror(errno));
    printError("{1}: {2}", &args);
  } else {
    args.push_back(strerror(errno));
    printError("{1}", &args);
  }
}


void bzfherror(const char* msg)
{
  std::vector<std::string> args;
  if (msg) {
    args.push_back(msg);
    args.push_back(hstrerror(h_errno));
    printError("{1}: {2}", &args);
  } else {
    args.push_back(hstrerror(h_errno));
    printError("{1}", &args);
  }
}


int getErrno()
{
  return errno;
}

} /* end 'extern "C"' */


int BzfNetwork::setNonBlocking(int fd)
{
  int mode = fcntl(fd, F_GETFL, 0);
  if (mode == -1 || fcntl(fd, F_SETFL, mode | O_NDELAY) < 0)
    return -1;
  return 0;
}


int BzfNetwork::setBlocking(int fd)
{
  int mode = fcntl(fd, F_GETFL, 0);
  if (mode == -1 || fcntl(fd, F_SETFL, mode & ~O_NDELAY) < 0)
    return -1;
  return 0;
}


BzfNetwork::ConnState BzfNetwork::getConnectionState(int fd)
{
  int optVal;
  socklen_t optLen = sizeof(optVal);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &optVal, &optLen) == -1) {
    return CONNSTATE_QUERY_FAILURE;
  }
  switch (optVal) {
    case 0: {
      return CONNSTATE_CONN_SUCCESS;
    }
    case EINPROGRESS:
    case EINTR: {
      return CONNSTATE_INPROGRESS;
    }
  }
  errno = optVal;
  return CONNSTATE_CONN_FAILURE;
}


int BzfNetwork::closeSocket(int fd)
{
  return close(fd);
}


//============================================================================//
#else /* defined(_WIN32) */
//============================================================================//

extern "C" {

int inet_aton(const char* cp, struct in_addr* pin)
{
  unsigned long a = inet_addr(cp);
  if (a == (unsigned long)-1) {
    // could be an error or cp could be a broadcast address.
    // FIXME -- this check is a little simplistic.
    if (strcmp(cp, "255.255.255.255") != 0) return 0;
  }

  pin->s_addr = a;
  return 1;
}


// win32 apparently cannot lookup error messages for us
static const struct { int code; const char* msg; } netErrorCodes[] = {
	/* 10004 */{WSAEINTR,		"The (blocking) call was canceled via WSACancelBlockingCall"},
	/* 10009 */{WSAEBADF,		"Bad file handle"},
	/* 10013 */{WSAEACCES,		"The requested address is a broadcast address, but the appropriate flag was not set"},
	/* 10014 */{WSAEFAULT,		"Invalid pointer value, or buffer is too small (WSAEFAULT)"},
	/* 10022 */{WSAEINVAL,		"Invalid argument (WSAEINVAL)"},
	/* 10024 */{WSAEMFILE,		"No more file descriptors available"},
	/* 10035 */{WSAEWOULDBLOCK,	"Socket is marked as non-blocking and no connections are present or the receive operation would block"},
	/* 10036 */{WSAEINPROGRESS,	"A blocking Windows Sockets operation is in progress"},
	/* 10037 */{WSAEALREADY,	"The asynchronous routine being canceled has already completed"},
	/* 10038 */{WSAENOTSOCK,	"At least on descriptor is not a socket"},
	/* 10039 */{WSAEDESTADDRREQ,	"A destination address is required"},
	/* 10040 */{WSAEMSGSIZE,	"The datagram was too large to fit into the specified buffer and was truncated"},
	/* 10041 */{WSAEPROTOTYPE,	"The specified protocol is the wrong type for this socket"},
	/* 10042 */{WSAENOPROTOOPT,	"The option is unknown or unsupported"},
	/* 10043 */{WSAEPROTONOSUPPORT, "The specified protocol is not supported"},
	/* 10044 */{WSAESOCKTNOSUPPORT, "The specified socket type is not supported by this address family"},
	/* 10045 */{WSAEOPNOTSUPP,	"The referenced socket is not a type that supports that operation"},
	/* 10046 */{WSAEPFNOSUPPORT,	"BSD: Protocol family not supported"},
	/* 10047 */{WSAEAFNOSUPPORT,	"The specified address family is not supported"},
	/* 10048 */{WSAEADDRINUSE,	"The specified address is already in use"},
	/* 10049 */{WSAEADDRNOTAVAIL,	"The specified address is not available from the local machine"},
	/* 10050 */{WSAENETDOWN,	"The Windows Sockets implementation has detected that the network subsystem has failed"},
	/* 10051 */{WSAENETUNREACH,	"The network can't be reached from this hos at this time"},
	/* 10052 */{WSAENETRESET,	"The connection must be reset because the Windows Sockets implementation dropped it"},
	/* 10053 */{WSAECONNABORTED,	"The virtual circuit was aborted due to timeout or other failure"},
	/* 10054 */{WSAECONNRESET,	"The virtual circuit was reset by the remote side"},
	/* 10055 */{WSAENOBUFS,		"No buffer space is available or a buffer deadlock has occured. The socket cannot be created"},
	/* 10056 */{WSAEISCONN,		"The socket is already connected"},
	/* 10057 */{WSAENOTCONN,	"The socket is not connected"},
	/* 10058 */{WSAESHUTDOWN,	"The socket has been shutdown"},
	/* 10059 */{WSAETOOMANYREFS,	"BSD: Too many references"},
	/* 10060 */{WSAETIMEDOUT,	"Attempt to connect timed out without establishing a connection"},
	/* 10061 */{WSAECONNREFUSED,	"The attempt to connect was forcefully rejected"},
	/* 10062 */{WSAELOOP,		"Undocumented WinSock error code used in BSD"},
	/* 10063 */{WSAENAMETOOLONG,	"Undocumented WinSock error code used in BSD"},
	/* 10064 */{WSAEHOSTDOWN,	"Host is down"},
	/* 10065 */{WSAEHOSTUNREACH,	"No route to host"},
	/* 10066 */{WSAENOTEMPTY,	"Undocumented WinSock error code"},
	/* 10067 */{WSAEPROCLIM,	"Too many processes"},
	/* 10068 */{WSAEUSERS,		"Undocumented WinSock error code"},
	/* 10069 */{WSAEDQUOT,		"Undocumented WinSock error code"},
	/* 10070 */{WSAESTALE,		"Undocumented WinSock error code"},
	/* 10071 */{WSAEREMOTE,		"Undocumented WinSock error code"},
	/* 10091 */{WSASYSNOTREADY,	"Underlying network subsytem is not ready for network communication"},
	/* 10092 */{WSAVERNOTSUPPORTED,	"The version of WinSock API support requested is not provided in this implementation"},
	/* 10093 */{WSANOTINITIALISED,	"WinSock subsystem not properly initialized"},
	/* 10101 */{WSAEDISCON,		"Virtual circuit has gracefully terminated connection"},
	/* 11001 */{WSAHOST_NOT_FOUND,	"Host not found"},
	/* 11002 */{WSATRY_AGAIN,	"Host not found"},
	/* 11003 */{WSANO_RECOVERY,	"Host name lookup error"},
	/* 11004 */{WSANO_DATA,		"No data for host"},
	/* end   */{0,			NULL}
};


const char* socket_strerror(int err)
{
  const char* errmsg = "<unknown winsock error>";
  for (int i = 0; netErrorCodes[i].code != 0; ++i) {
    if (netErrorCodes[i].code == err) {
      return netErrorCodes[i].msg;
    }
  }
  return errmsg;
}


void nerror(const char* msg)
{
  const int err = getErrno();
  const char* errmsg = socket_strerror(err);
  if (msg) {
    char buf[50];
    std::vector<std::string> args;
    args.push_back(msg);
    args.push_back(errmsg);
    sprintf(buf, "%d", err);
    args.push_back(buf);
    printError("{1}: {2} ({3})", &args);
  } else {
    char buf[50];
    std::vector<std::string> args;
    args.push_back(errmsg);
    sprintf(buf, "%d", err);
    args.push_back(buf);
    printError("{1} ({2})", &args);
  }
}


void herror(const char* msg)
{
  nerror(msg);
}


int getErrno()
{
  return WSAGetLastError();
}

} /* end 'extern "C"' */


int BzfNetwork::setNonBlocking(int fd)
{
  int on = 1;
  return ioctl(fd, FIONBIO, &on);
}


int BzfNetwork::setBlocking(int fd)
{
  int off = 0;
  return ioctl(fd, FIONBIO, &off);
}


BzfNetwork::ConnState BzfNetwork::getConnectionState(int fd)
{
  // reference:  http://msdn.microsoft.com/en-us/library/ms740141(VS.85).aspx
  struct timeval timeout = { 0, 1 };
  fd_set wrfds, exfds;
  FD_ZERO(&wrfds);
  FD_ZERO(&exfds);
  FD_SET(fd, &wrfds);
  FD_SET(fd, &exfds);
  const int selVal = select(fd + 1, NULL, &wrfds, &exfds, &timeout);
  if (selVal ==  0) { return CONNSTATE_INPROGRESS;    }
  if (selVal == -1) { return CONNSTATE_QUERY_FAILURE; }

  int optVal;
  int optLen = sizeof(optVal);
  if (getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&optVal, &optLen) == -1) {
    return CONNSTATE_QUERY_FAILURE;
  }
  if (optVal != 0) {
    WSASetLastError(optVal);
    return CONNSTATE_CONN_FAILURE;
  }

  return CONNSTATE_CONN_SUCCESS;
}


int BzfNetwork::closeSocket(int fd)
{
  return closesocket(fd);
}


//============================================================================//
#endif /* defined(_WIN32) */
//============================================================================//


// parse a url into its parts
bool BzfNetwork::parseURL(const std::string& url, std::string& protocol,
                          std::string& hostname, int& port,
                          std::string& pathname)
{
  static const char* defaultHostname = "localhost";

  std::string mungedurl;


  int delimiterpos = 0;
  for(; (delimiterpos < (int)url.length()) && (url[delimiterpos] != ':') && (url[delimiterpos] != ' '); delimiterpos++) {};
  if(url[delimiterpos] != ':')
    return false;

  // set defaults
  hostname = defaultHostname;

  // store protocol
  protocol = url.substr(0, delimiterpos);
  mungedurl = url.substr(delimiterpos + 1);


  // store hostname and optional port for some protocols
  if (protocol == "http") {
    if (mungedurl[0] == '/' && mungedurl[1] == '/') {
      mungedurl = mungedurl.substr(2);
      int pos = 0;
      for(; (pos < (int)mungedurl.length()) && (mungedurl[pos] != ':') && (mungedurl[pos] != '/') && (mungedurl[pos] != '\\') && (mungedurl[pos] != ' '); pos++) {};

      if(mungedurl[pos] == ' ')
	return false;

      if(pos != 0)
	hostname = mungedurl.substr(0, pos);

      mungedurl = mungedurl.substr(pos);
      pos = 0;

      if(mungedurl[0] == ':') {
	pos++;
	char portString[10];
	int i = 0;
	for(; isdigit(mungedurl[pos]) && i < 9; pos++)
	  portString[i++] = mungedurl[pos];
	portString[i] = '\0';
	port = atoi(portString);
      }

      mungedurl = mungedurl.substr(pos);

      if((mungedurl[0] != '\0') && (mungedurl[0] != '/') && (mungedurl[0] != '\\'))
	return false;

    }
  }
  if(mungedurl.length() != 0)
    pathname = mungedurl;
  else
    pathname = "";

  return true;
}


void setNoDelay(int fd)
{
  // turn off TCP delay (collection).  we want packets sent immediately.
#if defined(_WIN32)
  BOOL on = TRUE;
#else
  int on = 1;
#endif
  struct protoent *p = getprotobyname("tcp");
  if (p && setsockopt(fd, p->p_proto, TCP_NODELAY, (SSOType)&on, sizeof(on)) < 0) {
    nerror("enabling TCP_NODELAY");
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
