/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "PlayerInfo.h"

/* system implementation headers */
#include <assert.h>

void PlayerInfo::initPlayer(const struct sockaddr_in& clientAddr, int _fd) {
  AddrLen addr_len = sizeof(clientAddr);

  // store address information for player
  memcpy(&taddr, &clientAddr, addr_len);
  memcpy(&uaddr, &clientAddr, addr_len);

  // update player state
  time = TimeKeeper::getCurrent();
  fd = _fd;
  state = PlayerInLimbo;
  peer = Address(taddr);
  tcplen = 0;
  assert(outmsg == NULL);
  outmsgSize = 0;
  outmsgOffset = 0;
  outmsgCapacity = 0;
  paused = false;
#ifdef HAVE_ADNS_H
  if (adnsQuery) {
    adns_cancel(adnsQuery);
    adnsQuery = NULL;
  }
#endif
};

void PlayerInfo::resetPlayer() {
  wasRabbit = false;
  toBeKicked = false;
  Admin = false;

  passwordAttempts = 0;

  regName = callSign;
  makeupper(regName);

  accessInfo.explicitAllows.reset();
  accessInfo.explicitDenys.reset();
  accessInfo.verified = false;
  accessInfo.loginTime = TimeKeeper::getCurrent();
  accessInfo.loginAttempts = 0;
  accessInfo.groups.clear();
  accessInfo.groups.push_back("DEFAULT");
  lastRecvPacketNo = 0;
  lastSendPacketNo = 0;

  uqueue = NULL;
  dqueue = NULL;

  delayq.dequeuePackets();

  lagavg = 0;
  lagcount = 0;
  laglastwarn = 0;
  lagwarncount = 0;
  lagalpha = 1;

  jitteravg = 0;
  jitteralpha = 1;

  lostavg = 0;
  lostalpha = 1;

  lasttimestamp = 0.0f;
  lastupdate = TimeKeeper::getCurrent();
  lastmsg	 = TimeKeeper::getCurrent();

  nextping = TimeKeeper::getCurrent();
  nextping += 10.0;
  pingpending = false;
  pingseqno = 0;
  pingssent = 0;

#ifdef TIMELIMIT
  playedEarly = false;
#endif
}

bool PlayerInfo::isAccessVerified() const {
  return accessInfo.verified;
}

void PlayerInfo::resetAccess() {
  // shutdown TCP socket
  shutdown(fd, 2);
  close(fd);

  fd = -1;
  accessInfo.verified      = false;
  accessInfo.loginAttempts = 0;
  regName.empty();

  uqueue = NULL;
  dqueue = NULL;
  delayq.dequeuePackets();
  lastRecvPacketNo = 0;
  lastSendPacketNo = 0;
  // shutdown the UDP socket
  memset(&uaddr, 0, sizeof(uaddr));

  // no UDP connection anymore
  udpin = false;
  udpout = false;
  toBeKicked = false;

  tcplen = 0;

  callSign[0] = 0;

  if (outmsg != NULL) {
    delete[] outmsg;
    outmsg = NULL;
  }
  outmsgSize = 0;

  flagHistory.clear();

#ifdef HAVE_ADNS_H
  if (hostname) {
    free(hostname);
    hostname = NULL;
  }
#endif
}

bool PlayerInfo::gotAccessFailure() {
  return accessInfo.loginAttempts >= 5;
}

void PlayerInfo::setLoginFail() {
  accessInfo.loginAttempts++;
}

void PlayerInfo::setPermissionRights() {
  accessInfo.verified = true;
  // get their real info
  PlayerAccessInfo &info = getUserInfo(regName);
  accessInfo.explicitAllows = info.explicitAllows;
  accessInfo.explicitDenys = info.explicitDenys;
  accessInfo.groups = info.groups;
  DEBUG1("Identify %s\n", regName.c_str());
}

void PlayerInfo::reloadInfo() {
  if (accessInfo.verified && userExists(regName)) {
    accessInfo = getUserInfo(regName);
    accessInfo.verified = true;
  }
}

bool PlayerInfo::hasSetGroupPermission(const std::string& group) {
  return hasGroup(accessInfo, group);
}

bool PlayerInfo::hasPermission(PlayerAccessInfo::AccessPerm right) {
  return Admin || hasPerm(accessInfo, right);
}

void PlayerInfo::setGroup(const std::string& group) {
  addGroup(accessInfo, group);
}

void PlayerInfo::resetGroup(const std::string& group) {
  removeGroup(accessInfo, group);
}

void PlayerInfo::setAdmin() {
  passwordAttempts = 0;
  Admin = true;
}

void PlayerInfo::setRestartOnBase(bool on) {
  restartOnBase = on;
};

bool PlayerInfo::shouldRestartAtBase() {
  return restartOnBase;
};

std::string PlayerInfo::getName() {
  return regName;
};

bool PlayerInfo::isPasswordMatching(const char* pwd) {
  return verifyUserPassword(regName.c_str(), pwd);
};

bool PlayerInfo::isRegistered() const {
  return userExists(regName);
};

bool PlayerInfo::isExisting() {
  return fd != -1 && userExists(regName);
};

bool PlayerInfo::isIdentifyRequired() {
  return hasPerm(getUserInfo(regName), PlayerAccessInfo::requireIdentify);
};

bool PlayerInfo::isAllowedToEnter() {
  return accessInfo.verified || !isRegistered() || !isIdentifyRequired();
};

void PlayerInfo::storeInfo(const char* pwd) {
  PlayerAccessInfo info;
  info.groups.push_back("DEFAULT");
  info.groups.push_back("REGISTERED");
  std::string pass = pwd;
  setUserPassword(regName.c_str(), pass.c_str());
  setUserInfo(regName, info);
  DEBUG1("Register %s %s\n",regName.c_str(), pwd);
  updateDatabases();
}

void PlayerInfo::setPassword(const std::string&  pwd) {
  setUserPassword(regName.c_str(), pwd.c_str());
  updateDatabases();
}

uint8_t PlayerInfo::getPlayerProperties() {
  uint8_t result = 0;
  if (isRegistered())
    result |= IsRegistered;
  if (accessInfo.verified)
    result |= IsIdentified;
  if (Admin)
    result |= IsAdmin;
  return result;
};

#ifdef NETWORK_STATS
void PlayerInfo::initNetworkStatistics() {
  int i;
  struct MessageCount *statMsg;
  int direction;

  for (direction = 0; direction <= 1; direction++) {
    statMsg = msg[direction];
    for (i = 0; i < MessageTypes && statMsg[i].code != 0; i++) {
      statMsg[i].count = 0;
      statMsg[i].code = 0;
    }
    msgBytes[direction] = 0;
    perSecondTime[direction] = time;
    perSecondCurrentMsg[direction] = 0;
    perSecondMaxMsg[direction] = 0;
    perSecondCurrentBytes[direction] = 0;
    perSecondMaxBytes[direction] = 0;
  }
};

void PlayerInfo::dumpMessageStats() {
  int i;
  struct MessageCount *msgStats;
  int total;
  int direction;

  DEBUG1("Player connect time: %f\n", TimeKeeper::getCurrent() - time);
  for (direction = 0; direction <= 1; direction++) {
    total = 0;
    DEBUG1("Player messages %s:", direction ? "out" : "in");
    msgStats = msg[direction];
    for (i = 0; i < MessageTypes && msgStats[i].code != 0; i++) {
      DEBUG1(" %c%c:%u(%u)", msgStats[i].code >> 8, msgStats[i].code & 0xff,
	     msgStats[i].count, msgStats[i].maxSize);
      total += msgStats[i].count;
    }
    DEBUG1(" total:%u(%u) ", total, msgBytes[direction]);
    DEBUG1("max msgs/bytes per second: %u/%u\n",
	perSecondMaxMsg[direction],
	perSecondMaxBytes[direction]);
  }
  fflush(stdout);
}
#endif

bool PlayerInfo::isConnected() {
  return fd != -1;
};

int PlayerInfo::send(const void *buffer, size_t length) {
  int n;

  n = ::send(fd, (const char *)buffer, length, 0);
  if (n >= 0) 
    return n;

  // get error code
  const int err = getErrno();

  // if socket is closed then give up
  if (err == ECONNRESET || err == EPIPE) {
    return -1;
  }

  // just try again later if it's one of these errors
  if (err != EAGAIN && err != EINTR) {
    // dump other errors and remove the player
    nerror("error on write");
    toBeKicked = true;
    toBeKickedReason = "Write error";
  }
  return 0;
};

int PlayerInfo::bufferedSend(int playerIndex,
			     const void *buffer,
			     size_t length) {
  if (fd == -1)
    return 0;

  // try flushing buffered data
  if (outmsgSize != 0) {
    const int n = send(outmsg + outmsgOffset, outmsgSize);
    if (n == -1) {
      return -1;
    }
    if (n > 0) {
      outmsgOffset += n;
      outmsgSize   -= n;
    }
  }
  // if the buffer is empty try writing the data immediately
  if ((outmsgSize == 0) && length > 0) {
    const int n = send(buffer, length);
    if (n == -1) {
      return -1;
    }
    if (n > 0) {
      buffer  = (void*)(((const char*)buffer) + n);
      length -= n;
    }
  }
  // write leftover data to the buffer
  if (fd != -1 && length > 0) {
    // is there enough room in buffer?
    if (outmsgCapacity < outmsgSize + (int)length) {
      // double capacity until it's big enough
      int newCapacity = (outmsgCapacity == 0) ? 512 : outmsgCapacity;
      while (newCapacity < outmsgSize + (int)length)
	newCapacity <<= 1;

      // if the buffer is getting too big then drop the player.  chances
      // are the network is down or too unreliable to that player.
      // FIXME -- is 20kB too big?  too small?
      if (newCapacity >= 20 * 1024) {
	DEBUG2("Player %s [%d] drop, unresponsive with %d bytes queued\n",
	       callSign, playerIndex, outmsgSize + length);
	toBeKicked = true;
	toBeKickedReason = "send queue too big";
	return 0;
      }

      // allocate memory
      char *newbuf = new char[newCapacity];

      // copy old data over
      memmove(newbuf, outmsg + outmsgOffset, outmsgSize);

      // cutover
      delete[] outmsg;
      outmsg	       = newbuf;
      outmsgOffset   = 0;
      outmsgCapacity = newCapacity;
    }

    // if we can't fit new data at the end of the buffer then move existing
    // data to head of buffer
    // FIXME -- use a ring buffer to avoid moving memory
    if (outmsgOffset + outmsgSize + (int)length > outmsgCapacity) {
      memmove(outmsg, outmsg + outmsgOffset, outmsgSize);
      outmsgOffset = 0;
    }

    // append data
    memmove(outmsg + outmsgOffset + outmsgSize, buffer, length);
    outmsgSize += length;
  }
  return 0;
};

RxStatus PlayerInfo::receive(size_t length) {
  RxStatus returnValue;
  if (fd == -1)
    return ReadPart;
  if ((int)length <= tcplen)
    return ReadAll;
  int size = recv(fd, tcpmsg + tcplen, length - tcplen, 0);
  if (size > 0) {
    tcplen += size;
    if (tcplen == (int)length)
      returnValue = ReadAll;
    else
      returnValue = ReadPart;
  } else if (size < 0) {
    // handle errors
    // get error code
    const int err = getErrno();

    // ignore if it's one of these errors
    if (err == EAGAIN || err == EINTR)
      returnValue = ReadPart;

    // if socket is closed then give up
    if (err == ECONNRESET || err == EPIPE) {
      returnValue = ReadReset;
    } else {
      returnValue = ReadError;
    }
  } else { // if (size == 0)
    returnValue = ReadDiscon;
  }
  return returnValue;
};

void PlayerInfo::resetComm() {
    fd = -1;
    state = PlayerNoExist;
    delayq.init();
    outmsg = NULL;
    outmsgSize = 0;
    outmsgOffset = 0;
    outmsgCapacity = 0;
#ifdef HAVE_ADNS_H
    hostname = NULL;
    adnsQuery = NULL;
#endif  
};

void PlayerInfo::closeComm() {
  if (fd != -1) {
    shutdown(fd, 2);
    close(fd);
    delete [] outmsg;
  }
};

void PlayerInfo::dropUnconnected() {
  if (fd == -1)
    state = PlayerNoExist;
};

void PlayerInfo::debugRemove(const char *reason, int index) {
  // status message
  DEBUG1("Player %s [%d] on %d removed: %s\n", callSign, index, fd, reason);
};

void PlayerInfo::debugAdd(int index) {
  DEBUG1("Player %s [%d] has joined from %s:%d on %i\n",
	 callSign, index, inet_ntoa(taddr.sin_addr),
	 ntohs(taddr.sin_port), fd);
};

void PlayerInfo::fdSet(fd_set *read_set, fd_set *write_set, int &maxFile) {
  if (fd != -1) {
    _FD_SET(fd, read_set);

    if (outmsgSize > 0)
      _FD_SET(fd, write_set);
    if (fd > maxFile)
      maxFile = fd;
  }
};

int PlayerInfo::fdIsSet(fd_set *set) {
  return FD_ISSET(fd, set);
}

void PlayerInfo::debugPwdTries() {
  DEBUG1("%s (%s) has attempted too many /password tries\n",
	 callSign, peer.getDotNotation().c_str());
};

void PlayerInfo::getPlayerList(char *list, int index) {
  sprintf(list, "[%d]%-16s: %s%s%s%s%s%s", index, callSign,
	  peer.getDotNotation().c_str(),
#ifdef HAVE_ADNS_H
	  hostname ? " (" : "",
	  hostname ? player[i].hostname : "",
	  hostname ? ")" : "",
#else
	  "", "", "",
#endif
	  udpin ? " udp" : "",
	  udpout ? "+" : "");
}; 

const char *PlayerInfo::getTargetIP() {
  return peer.getDotNotation().c_str();
};

int PlayerInfo::sizeOfIP() {
   return peer.getIPVersion() == 4 ? 8 : 20; // 8 for IPv4, 20 for IPv6
};

void *PlayerInfo::packAdminInfo(void *buf, int index) {
  buf = nboPackUByte(buf, sizeOfIP());
  buf = nboPackUByte(buf, index);
  buf = nboPackUByte(buf, getPlayerProperties());
  buf = peer.pack(buf);
  return buf;
};

void PlayerInfo::debugUnknownPacket(int index, int code) {
  DEBUG1("Player [%d] sent unknown packet type (%x), \
possible attack from %s\n",
	 index,code,peer.getDotNotation().c_str());
};

bool PlayerInfo::isAtIP(const std::string& IP) {
  return strcmp(peer.getDotNotation().c_str(), IP.c_str()) == 0;
};

void PlayerInfo::debugHugePacket(int index, int length) {
  DEBUG1("Player [%d] sent huge packet length (len=%d), \
possible attack from %s\n",
	 index, length, peer.getDotNotation().c_str());
};

bool PlayerInfo::isPlaying() {
  return state > PlayerInLimbo;
};

bool PlayerInfo::exist() {
  return state != PlayerNoExist;
};
void PlayerInfo::signingOn() {
  state = PlayerDead;
  flag = -1;
  wins = 0;
  losses = 0;
  tks = 0;  
};

bool PlayerInfo::isAlive() {
  return state == PlayerAlive;
};

bool PlayerInfo::isInLimbo() {
  return state == PlayerInLimbo;
};

void PlayerInfo::remove() {
  state = PlayerNoExist;
};

bool PlayerInfo::isDead() {
  return state == PlayerDead;
};

void PlayerInfo::setAlive() {
  state = PlayerAlive;
  flag = -1;
};

void PlayerInfo::setDead() {
  state = PlayerDead;
};

bool PlayerInfo::isBot() {
  return type == ComputerPlayer;
};

bool PlayerInfo::isHuman() {
  return type == TankPlayer;
};

void *PlayerInfo::packUpdate(void *buf) {
  buf = nboPackUShort(buf, uint16_t(type));
  buf = nboPackUShort(buf, uint16_t(team));
  buf = nboPackUShort(buf, uint16_t(wins));
  buf = nboPackUShort(buf, uint16_t(losses));
  buf = nboPackUShort(buf, uint16_t(tks));
  buf = nboPackString(buf, callSign, CallSignLen);
  buf = nboPackString(buf, email, EmailLen);
  return buf;
};

void *PlayerInfo::unpackEnter(void *buf) {
  // data: type, team, name, email
  uint16_t _type;
  int16_t _team;
  buf = nboUnpackUShort(buf, _type);
  buf = nboUnpackShort(buf, _team);
  type = PlayerType(_type);
  team = TeamColor(_team);
  buf = nboUnpackString(buf, callSign, CallSignLen);
  buf = nboUnpackString(buf, email, EmailLen);
  return buf;
};

void PlayerInfo::getLagStats(char* msg) {
  if ((state > PlayerInLimbo) && (type == TankPlayer)) {
    sprintf(msg,"%-16s : %3d +- %2dms %s", callSign,
	    int(lagavg * 1000),
	    int(jitteravg * 1000),
	    accessInfo.verified ? "(R)" : "");
    if (lostavg >= 0.01f)
      sprintf(msg + strlen(msg), " %d%% lost/ooo", int(lostavg * 100));
  } else {
    msg[0] = 0;
  }
};

const char *PlayerInfo::getCallSign() const {
  return callSign;
};

void PlayerInfo::cleanCallSign() {
  // strip leading whitespace from callsign
  char *sp = callSign;
  char *tp = sp;
  while (isspace(*sp))
    sp++;

  // strip any non-printable characters and ' and " from callsign
  do {
    if (isprint(*sp) && (*sp != '\'') && (*sp != '"')) {
      *tp++ = *sp;
    }
  } while (*++sp);
  *tp = *sp;

  // strip trailing whitespace from callsign
  while (isspace(*--tp)) {
    *tp=0;
  }
};

bool PlayerInfo::isCallSignReadable() {
  // callsign readability filter, make sure there are more alphanum than non
  // keep a count of alpha-numerics
  int alnumCount = 0;
  const char *sp = callSign;
  do {
    if (isalnum(*sp)) {
      alnumCount++;
    }
  } while (*++sp);
  int callsignlen = strlen(callSign);
  return (callsignlen <= 4) || ((float)alnumCount / (float)callsignlen > 0.5f);
};

const char *PlayerInfo::getEMail() const {
  return email;
};

void PlayerInfo::cleanEMail() {
  // strip leading whitespace from email
  char *sp = email;
  char *tp = sp;
  while (isspace(*sp))
    sp++;

  // strip any non-printable characters and ' and " from email
  do {
    if (isprint(*sp) && (*sp != '\'') && (*sp != '"')) {
      *tp++ = *sp;
    }
  } while (*++sp);
  *tp = *sp;

  // strip trailing whitespace from email
  while (isspace(*--tp)) {
    *tp=0;
  }
};

bool PlayerInfo::isEMailReadable() {
  // email/"team" readability filter, make sure there are more
  // alphanum than non
  int emailAlnumCount = 0;
  char *sp = email;
  do {
    if (isalnum(*sp)) {
      emailAlnumCount++;
    }
  } while (*++sp);
  int emaillen = strlen(email);
  return (emaillen <= 4) || (((float)emailAlnumCount / (float)emaillen) > 0.5);
};

void *PlayerInfo::packVirtualFlagCapture(void *buf) {
  buf = nboPackUShort(buf, uint16_t(int(team) - 1));
  buf = nboPackUShort(buf, uint16_t(1 + (int(team) % 4)));
  return buf;
};

bool PlayerInfo::isTeam(TeamColor _team) const {
  return team == _team;
};

bool PlayerInfo::isObserver() const {
  return team == ObserverTeam;
};

TeamColor PlayerInfo::getTeam() {
  return team;
};

void PlayerInfo::setTeam(TeamColor _team) {
  team = _team;
};

void PlayerInfo::wasARabbit() {
  team = RogueTeam;
  wasRabbit = true;
};

void PlayerInfo::wasNotARabbit() {
  wasRabbit = false;
};

bool PlayerInfo::isARabbitKill(PlayerInfo &victim) {
  return wasRabbit || victim.team == RabbitTeam;
};

void PlayerInfo::resetFlag() {
  flag = -1;
  lastFlagDropTime = TimeKeeper::getCurrent();
};

bool PlayerInfo::haveFlag() const {
  return flag >= 0;
}
int PlayerInfo::getFlag() const {
  return flag;
};

void PlayerInfo::setFlag(int _flag) {
  flag = _flag;
};

void PlayerInfo::dumpScore() {
  if (state > PlayerInLimbo)
    std::cout << wins << '-' << losses << ' ' << callSign << std::endl;
};

float PlayerInfo::scoreRanking() {
  int sum = wins + losses;
  if (sum == 0)
    return 0.5;
  float average = (float)wins/(float)sum;
  // IIRC that is how wide is the gaussian
  float penalty = (1.0f - 0.5f / sqrt((float)sum));
  return average * penalty;
};

bool PlayerInfo::setAndTestTK(float tkKickRatio) {
  tks++;
  return (tks >= 3) && (tkKickRatio > 0) && // arbitrary 3
    ((wins == 0) || (tks * 100 / wins > tkKickRatio));
};

void PlayerInfo::setOneMoreLoss() {
  losses++;
};

void PlayerInfo::setOneMoreWin() {
  wins++;
};

void *PlayerInfo::packScore(void *buf, int index) {
  buf = nboPackUByte(buf, index);
  buf = nboPackUShort(buf, wins);
  buf = nboPackUShort(buf, losses);
  buf = nboPackUShort(buf, tks);
  return buf;
};

bool PlayerInfo::scoreReached(int score) {
  return wins - losses >= score;
};

bool PlayerInfo::isFlagTransitSafe() {
  return TimeKeeper::getCurrent() - lastFlagDropTime >= 2.0f;
};

void PlayerInfo::udpFillRead(void *buf, int len) {
  if (len >= 1024)
    len = 1023;
  memcpy(udpmsg, buf, len);
};

void *PlayerInfo::getUdpBuffer() {
  return udpmsg;
};

void *PlayerInfo::getTcpBuffer() {
  return tcpmsg;
};

void PlayerInfo::cleanTcp() {
  tcplen = 0;
};
// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
