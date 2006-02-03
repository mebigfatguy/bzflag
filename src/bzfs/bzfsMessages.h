/* bzflag
* Copyright (c) 1993 - 2006 Tim Riker
*
* This package is free software;  you can redistribute it and/or
* modify it under the terms of the license found in the file
* named COPYING that should have accompanied this file.
*
* THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifndef _BZFS_MESSAGES_H_
#define _BZFS_MESSAGES_H_

#include "global.h"
#include "GameKeeper.h"
#include "bzfs.h"

// general messages sent to players ( local and remove )
void sendRemovePlayerMessage ( int playerID );
void sendFlagUpdateMessage ( int playerID );
void sendExistingPlayerUpdates ( int newPlayer );

// messages sent to just network users ( like client query )
int sendPlayerUpdateDirect(NetHandler *handler, GameKeeper::Player *otherData);

#endif //_BZFS_MESSAGES_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8