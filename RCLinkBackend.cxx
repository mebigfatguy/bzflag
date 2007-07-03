/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <stdarg.h>

#include "RCLinkBackend.h"

#include "version.h"

RCLinkBackend::RCLinkBackend(int _port) :RCLink(),
                                        requests(NULL)
{
  port = _port;
  startListening();
}

RCLink::State RCLinkBackend::getDisconnectedState()
{
    return RCLink::Listening;
}


/*
 * Check for activity.  If possible, fill up the recvbuf with incoming data
 * and build up RCRequest objects as appropriate.
 */
void RCLinkBackend::update()
{
  if (status != Connected && status != Connecting) {
    return;
  }

  updateWrite();
  int amount = updateRead();

  if (amount == -1) {
    status = Listening;
    return;
  }

  if (status == Connected) {
    updateParse();
  } else if (status == Connecting) {
    int ncommands = updateParse(1);
    if (ncommands) {
      RCRequest *req = popRequest();
      if (req && req->getType() == "IdentifyFrontend") {
        status = Connected;
        req->sendAck();
      } else {
        fprintf(stderr, "RCLink: Expected an 'IdentifyFrontend'.\n");
        write(connfd, RC_LINK_NOIDENTIFY_MSG, strlen(RC_LINK_NOIDENTIFY_MSG));
        close(connfd);
        status = Listening;
      }
    }
  }
}

/*
 * Parse a command, create an RCRequest object, and add it to requests.
 * Return true if an RCRequest was successfully created.  If it failed,
 * return false.
 */
bool RCLinkBackend::parseCommand(char *cmdline)
{
  RCRequest *req;
  int argc;
  char *argv[RC_LINK_MAXARGS];
  char *s, *tkn;

  s = cmdline;
  for (argc = 0; argc < RC_LINK_MAXARGS; argc++) {
    tkn = strtok(s, " \r\t");
    s = NULL;
    argv[argc] = tkn;
    if (tkn == NULL || *tkn == '\0')
      break;
  }

  req = RCRequest::getInstance(argv[0], this);
  if (req == NULL) {
    fprintf(stderr, "RCLink: Invalid request: '%s'\n", argv[0]);
    sendf("error Invalid request %s\n", argv[0]);
    return false;
  } else {
    switch (req->parse(argv + 1, argc - 1))
    {
      case RCRequest::ParseOk:
        if (requests == NULL)
          requests = req;
        else
          requests->append(req);
        return true;
      case RCRequest::InvalidArgumentCount:
        fprintf(stderr, "RCLink: Invalid number of arguments (%d) for request: '%s'\n", argc - 1, argv[0]);
        sendf("error Invalid number of arguments (%d) for request: '%s'\n", argc - 1, argv[0]);
        return false;
      case RCRequest::InvalidArguments:
        fprintf(stderr, "RCLink: Invalid arguments for request: '%s'\n", argv[0]);
        sendf("error Invalid arguments for request: '%s'\n", argv[0]);
        return false;
      default:
        fprintf(stderr, "RCLink: Parse neither succeeded or failed with a known failcode. WTF?\n");
        return false;
    }
  }
}

RCRequest* RCLinkBackend::popRequest()
{
  RCRequest *req = requests;
  if (req != NULL) {
    requests = req->getNext();
  }
  return req;
}
RCRequest* RCLinkBackend::peekRequest()
{
  return requests;
}

bool RCLinkBackend::tryAccept()
{
  if (!RCLink::tryAccept())
    return false;

  write(connfd, RC_LINK_IDENTIFY_STR, strlen(RC_LINK_IDENTIFY_STR));
  write(connfd, getRobotsProtocolVersion(), strlen(getRobotsProtocolVersion()));
  write(connfd, "\n", 1);

  return true;
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
