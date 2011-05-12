/* bzflag
* Copyright (c) 1993 - 2008 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#include "EventHandler.h"
#include "TimeKeeper.h"

INSTANTIATE_SINGLETON(EventHandler)

EventHandler::EventHandler()
{
}

EventHandler::~EventHandler()
{
  // the events delink themselves when deleted
  while(!timeMap.empty()) {
    TimeMapType::iterator itr = timeMap.begin();
    itr->second->cancel();
    delete itr->second;
  }
}

void EventHandler::update()
{
  double now = TimeKeeper::getTick().getSeconds();
   
  while(!timeMap.empty() && timeMap.begin()->first <= now) {
    Event *e = timeMap.begin()->second;
    e->call();
    e->delink();
    if(!e->refCounter) delete e;
  }
}

void EventHandler::addDelta(CBFunc execFunc, void * data, double delta, CBFunc cancelFunc)
{
  double now = TimeKeeper::getTick().getSeconds();
  TimeMapType::iterator itr = timeMap.lower_bound(now + delta);
  itr = timeMap.insert(itr, TimeMapType::value_type(now + delta, NULL));
  itr->second = new Event(execFunc, cancelFunc, data, itr);
}

EventHandler::Event::Event(CBFunc ef, CBFunc cf, void *d, TimeMapType::iterator &i)
  : refCounter(1), execFunc(ef), cancelFunc(cf), data(d), itr(i)
{
}

void EventHandler::Event::delink()
{
  if(itr != sEventHandler.timeMap.end()) {
    sEventHandler.timeMap.erase(itr);
    itr = sEventHandler.timeMap.end();
    refCounter--;
  }
}

EventHandler::Event::~Event()
{
  delink();
}

void EventHandler::Event::call()
{
  (*execFunc)(data);
}

void EventHandler::Event::cancel()
{
  (*cancelFunc)(data);
}

void EventHandler::cancelEvent(void * data)
{
  delete data;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
