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

#ifndef _BZFS_CLIENT_MESSAGES_H_
#define _BZFS_CLIENT_MESSAGES_H_

#include "global.h"
#include "bzfs.h"

void handleClientEnter ( void **buf, GameKeeper::Player *playerData );
void handleClientExit ( GameKeeper::Player *playerData );
void handleSetVar ( NetHandler *netHandler );
void handleFlagNegotiation( NetHandler *handler, void **buf, int len );
void handleWorldChunk( NetHandler *handler, void *buf );
void handleWorldSettings( NetHandler *handler );
void handleWorldHash( NetHandler *handler );
void handlePlayerKilled( GameKeeper::Player *playerData, void* buffer );
void handlePlayerFlagDrop( GameKeeper::Player *playerData, void* buffer );
void handleGameJoinRequest( GameKeeper::Player *playerData );
void handlePlayerUpdate ( void **buf, uint16_t &code, GameKeeper::Player *playerData, const void* rawbuf, int len );	//once relay is based on state, remove rawbuf
void handlePlayerMessage ( GameKeeper::Player *playerData, void* buffer );
void handleFlagCapture ( GameKeeper::Player *playerData, void* buffer);

// util functions 
bool updatePlayerState(GameKeeper::Player *playerData, PlayerState &state, float timeStamp, bool shortState);

#endif //_BZFS_CLIENT_MESSAGES_H_


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
