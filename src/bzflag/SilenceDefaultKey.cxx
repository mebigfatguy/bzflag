/* bzflag
 * Copyright (c) 1993 - 2004 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <string>

#include "SilenceDefaultKey.h"
#include "KeyManager.h"
#include "LocalPlayer.h"
#include "HUDRenderer.h"
#include "Roster.h"

#include "playing.h" // THIS IS TEMPORARY..TO BE REMOVED..BABY STEPS

//
// Choose person to silence

SilenceDefaultKey::SilenceDefaultKey()
{
}

bool			SilenceDefaultKey::keyPress(const BzfKeyEvent& key)
{
  bool sendIt;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (KEYMGR.get(key, true) == "jump") {
    // jump while typing
    myTank->setJump();
  }

  if (myTank->getInputMethod() != LocalPlayer::Keyboard) {
    if ((key.button == BzfKeyEvent::Up) ||
	(key.button == BzfKeyEvent::Down) ||
	(key.button == BzfKeyEvent::Left) ||
	(key.button == BzfKeyEvent::Right))

      return true;
  }

  switch (key.ascii) {
  case 3:	// ^C
  case 27:	// escape
    //    case 127:	// delete
    sendIt = false;			// finished composing -- don't send
    break;

  case 4:	// ^D
  case 13:	// return
    sendIt = true;
    break;

  default:
    return false;
  }

  if (sendIt) {
    std::string message = hud->getComposeString();

    // find the name of the person to silence,
    // either by picking through arrow keys or by compose
    const char* name = NULL;

    if (message.size() == 0) {
      // silence just by picking arrowkeys
      const Player * silenceMe = myTank->getRecipient();
      if (silenceMe) {
	name = silenceMe->getCallSign();
      }
    }
    else if (message.size() > 0) {
      // typed in name
      name = message.c_str();
    }

    // if name is NULL we skip
    if (name != NULL) {
      // bad indent :)
      int inListPos = -1;
      for (unsigned int i = 0; i < silencePlayers.size(); i++) {
	if (strcmp(silencePlayers[i].c_str(),name) == 0) {
	  inListPos = i;
	}
      }

      bool isInList = (inListPos != -1);

      Player *loudmouth = getPlayerByName(name);
      if (loudmouth) {
	// we know this person exists
	if (!isInList) {
	  // exists and not in silence list
	  silencePlayers.push_back(name);
	  std::string message = "Silenced ";
	  message += (name);
	  addMessage(NULL, message);
	} else {
	  // exists and in list --> remove from list
	  silencePlayers.erase(silencePlayers.begin() + inListPos);
	  std::string message = "Unsilenced ";
	  message += (name);
	  addMessage(NULL, message);
	}
      } else {
	// person does not exist, but may be in silence list
	if (isInList) {
	  // does not exist but is in list --> remove
	  silencePlayers.erase(silencePlayers.begin() + inListPos);
	  std::string message = "Unsilenced ";
	  message += (name);
	  if (strcmp (name, "*") == 0) {
	    // to make msg fancier
	    message = "Unblocked Msgs";
	  }
	  addMessage(NULL, message);
	} else {
	  // does not exist and not in list -- duh
	  if (name != NULL) {
	    if (strcmp (name,"*") == 0) {
	      // check for * case
	      silencePlayers.push_back(name);
	      std::string message = "Silenced All Msgs";
	      addMessage(NULL, message);
	    } else {
	      std::string message = name;
	      message += (" Does not exist");
	      addMessage(NULL, message);
	    }
	  }
	}
      }
    }
  }

  hud->setComposing(std::string());

  HUDui::setDefaultKey(NULL);
  return true;
}

bool			SilenceDefaultKey::keyRelease(const BzfKeyEvent& key)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  if (myTank->getInputMethod() != LocalPlayer::Keyboard) {

    if (key.button == BzfKeyEvent::Up || key.button==BzfKeyEvent::Down
	||key.button==BzfKeyEvent::Left||key.button==BzfKeyEvent::Right) {
      // exclude robots from silence recipient list they don't talk
      selectNextRecipient(key.button == BzfKeyEvent::Up ||
			  key.button == BzfKeyEvent::Right, false);
      const Player *recipient = myTank->getRecipient();
      if (recipient) {
	const std::string name = recipient->getCallSign();
	bool isInList = false;
	for (unsigned int i = 0; i < silencePlayers.size(); i++) {
	  if (silencePlayers[i] == name) {
	    isInList = true;
	    break;
	  }
	}
	std::string composePrompt = "Silence -->";
	if (isInList) composePrompt = "Un" + composePrompt;
	composePrompt += name;

	// Set the prompt and disable editing/composing
	hud->setComposing(composePrompt, false);
      }
      return false;
    }
  }
  return keyPress(key);
}
