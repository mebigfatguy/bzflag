/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* no header other than VotingArbiter.h should be included here */

#ifdef _WIN32
#pragma warning( 4:4786)
#endif

#include "VotingArbiter.h"


/* private */

/* protected */

void VotingArbiter::updatePollers(void)
{
  while (!_pollers.empty()) {
    poller_t& p = _pollers.front();
    if ((TimeKeeper::getCurrent() - p.lastRequest) > _voteRepeatTime) {
      // remove pollers that expired their repoll timeout
      _pollers.pop_front();
    } else {
      break;
    }
  }
  return;
}

bool VotingArbiter::isPollerWaiting(std::string name) const
{
  for (unsigned int i = 0; i < _pollers.size(); i++) {
    if (_pollers[i].name == name) {
      return true;
    }
  }
  return false;
}


/* public */

bool VotingArbiter::forgetPoll(void) 
{
  if (_votingBooth != NULL) {
    delete _votingBooth;
    _votingBooth = NULL;
  }
  _startTime = TimeKeeper::getNullTime();
  _pollee = "nobody";
  _polleeIP = "";
  _action = UNDEFINED;
  _pollRequestor = "nobody";

  /* poof */
  _suffraged.clear();

  return true;
}

bool VotingArbiter::poll(std::string player, std::string playerRequesting, pollAction_t action, std::string playerIP) 
{
  poller_t p;
  char message[256];
  bool tooSoon;
  
  // you have to forget the current poll before another can be spawned
  if (this->isPollOpen()) {
    return false;
  }

  // update the poller list (people on the pollers list cannot initiate a poll
  updatePollers();

  // see if the poller is in the list
  tooSoon = isPollerWaiting(playerRequesting);
  if (tooSoon) {
    return false;
  }
  
  // add this poller to the end list
  p.name = playerRequesting;
  p.lastRequest = TimeKeeper::getCurrent();
  _pollers.push_back(p);

  // create the booth to record votes
#ifdef _WIN32
  _snprintf(message, 256,  "%s %s", action == POLL_KICK_PLAYER ? "kick" : "ban", player.c_str());
#else
  snprintf(message, 256,  "%s %s", action == POLL_KICK_PLAYER ? "kick" : "ban", player.c_str());
#endif
  if (_votingBooth != NULL) {
    delete _votingBooth;
  }
  _votingBooth = YesNoVotingBooth(message);
  _pollee = player;
  _polleeIP = playerIP;
  _action = action;
  _pollRequestor = playerRequesting;

  // set timers
  _startTime = TimeKeeper::getCurrent();

  return true;
}

bool VotingArbiter::pollToKick(std::string player, std::string playerRequesting)
{
  return (this->poll(player, playerRequesting, POLL_KICK_PLAYER));
}

bool VotingArbiter::pollToBan(std::string player, std::string playerRequesting, std::string playerIP)
{
  return (this->poll(player, playerRequesting, POLL_BAN_PLAYER, playerIP));
}

bool VotingArbiter::closePoll(void)
{
  if (this->isPollClosed()) {
    return true;
  }
  // set starting time to exactly current time minus necessary vote time
  _startTime = TimeKeeper::getCurrent();
  _startTime += -(_voteTime);

  return true;
}
      
bool VotingArbiter::setAvailableVoters(unsigned short int count) 
{
  _maxVotes = count;
  return true;
}

bool VotingArbiter::grantSuffrage(std::string player)
{
  for (unsigned int i = 0; i < _suffraged.size(); i++) {
    if (_suffraged[i] == player) {
      return true;
    }
  }
  _suffraged.push_front(player);
  return true;
}

bool VotingArbiter::hasSuffrage(std::string player) const
{
  // is there a poll to vote on?
  if (!this->isPollOpen()) {
    return false;
  }

  // was this player granted the right to vote?
  bool foundPlayer = false;
  for (unsigned int i = 0; i < _suffraged.size(); i++) {
    if (_suffraged[i] == player) {
      foundPlayer = true;
      break;
    }
  }
  if (!foundPlayer) {
    return false;
  }

  // has this player already voted?
  if (_votingBooth->hasVoted(player)) {
    return false;
  }

  // are there too many votes somehow (sanity)
  if (_votingBooth->getTotalVotes() >= _maxVotes) {
    return false;
  }
  
  return true;
}

bool VotingArbiter::voteYes(std::string player) 
{
  if (!this->knowsPoll() || this->isPollClosed()) {
    return false;
  }

  // allowed to vote?
  if (!hasSuffrage(player)) {
    return false;
  }

  return (_votingBooth->vote(player, 1));
}

bool VotingArbiter::voteNo(std::string player) 
{
  if (!this->knowsPoll() || this->isPollClosed()) {
    return false;
  }

  // allowed to vote?
  if (!hasSuffrage(player)) {
    return false;
  }

  return (_votingBooth->vote(player, 0));
}

unsigned long int VotingArbiter::getYesCount(void) const
{
  if (!this->knowsPoll()) {
    return 0;
  }
  return _votingBooth->getVoteCount(1);
}

unsigned long int VotingArbiter::getNoCount(void) const
{ 
  if (!this->knowsPoll()) {
    return 0;
  }
  return _votingBooth->getVoteCount(0);
}

unsigned long int VotingArbiter::getAbstentionCount(void) const
{
  // cannot abstain if there is no poll
  if (!this->knowsPoll()) {
    return 0;
  }
  int count = _suffraged.size() - this->getYesCount() - this->getNoCount();
  if (count <= 0) {
    return 0;
  }
  return count;
}

bool VotingArbiter::isPollSuccessful(void) const 
{
  if (!this->knowsPoll()) {
    return false;
  }
  unsigned long int votes = _votingBooth->getVoteCount(1);

  //ensure minimum votage
  if (votes < _votesRequired) {
    return false;
  }

  //  std::cout << "Percentage successful is " << ((double)votes * (double)100.0 / (double)_maxVotes) << " with " << votes << " votes and " << _maxVotes << "max votes required" << std::endl;

  // were there enough votes?
  if (((double)votes * (double)100.0 / (double)_maxVotes) > (double)_votePercentage) {
    return true;
  }
  
  return false;
}

unsigned long int VotingArbiter::timeRemaining(void) const
{
  if (_votingBooth == NULL) {
    return 0;
  }

  // if the poll is successful early, terminate the clock
  if (this->isPollSuccessful()) {
    return 0;
  }
  
  float remaining = _voteTime - (TimeKeeper::getCurrent() - _startTime);
  if (remaining < 0.0f) {
    return 0;
  }
  return (unsigned int)remaining;
}

bool VotingArbiter::retractVote(std::string player) 
{
  if (_votingBooth == NULL) {
    return false;
  }
  return _votingBooth->retractVote(player);
}

// ex: shiftwidth=2 tabstop=8
