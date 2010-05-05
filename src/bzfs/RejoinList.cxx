/* bzflag
 * Copyright (c) 1993-2010 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// interface header
#include "RejoinList.h"

// common headers
#include "BzTime.h"
#include "PlayerInfo.h"
#include "StateDatabase.h"  // for BZDBNAMES.REJOINTIME

// bzfs specific headers
#include "CmdLineOptions.h" // for MaxPlayers & ReplayObservers
#include "RecordReplay.h"
#include "GameKeeper.h"

// it's loathsome to expose private structure in a header
struct RejoinNode {
  char callsign[CallSignLen];
  BzTime joinTime;
};


RejoinList::RejoinList()
{
  queue.clear(); // call me paranoid
}

RejoinList::~RejoinList()
{
  std::list<struct RejoinNode*>::iterator it;
  for (it = queue.begin(); it != queue.end(); it++) {
    RejoinNode *rn = *it;
    delete rn;
  }
  queue.clear();
}


bool RejoinList::add(int playerIndex)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  if (playerData == NULL) {
    return false;
  }
  RejoinNode* rn = new RejoinNode;
  strncpy (rn->callsign, playerData->player.getCallSign(), CallSignLen-1);
  rn->joinTime = BzTime::getCurrent();
  queue.push_back (rn);
  return true;
}


void RejoinList::remove(int playerIndex)
{
  GameKeeper::Player *playerData = GameKeeper::Player::getPlayerByIndex(playerIndex);
  const char *callsign = playerData->player.getCallSign();
  if (!playerData || !callsign) {
    return;
  }

  std::list<struct RejoinNode*>::iterator it;
  it = queue.begin();
  while (it != queue.end()) {
    RejoinNode *rn = *it;
    if (strcasecmp (rn->callsign, callsign) == 0) {
      delete rn;
      it = queue.erase(it);
      return;
    }
    it++;
  }
}


float RejoinList::waitTime(int playerIndex)
{
  GameKeeper::Player *playerData
    = GameKeeper::Player::getPlayerByIndex(playerIndex);
  const char *callsign = playerData->player.getCallSign();
  if (!playerData || !callsign) {
    return 0.0f;
  }

  std::list<struct RejoinNode*>::iterator it;
  BzTime thenTime = BzTime::getCurrent();
  thenTime += -BZDB.eval(BZDBNAMES.REJOINTIME);

  // remove old entries
  it = queue.begin();
  while (it != queue.end()) {
    RejoinNode *rn = *it;
    if (rn->joinTime <= thenTime) {
      delete rn;
      it = queue.erase(it);
      continue;
    }
    it++;
  }

  float value = 0.0f;
  it = queue.begin();
  while (it != queue.end()) {
    RejoinNode *rn = *it;
    if (strcasecmp (rn->callsign, callsign) == 0) {
      value = float(rn->joinTime - thenTime);
    }
    it++;
  }

  return value;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
