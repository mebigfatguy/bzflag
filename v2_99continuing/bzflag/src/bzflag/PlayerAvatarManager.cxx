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
#include "PlayerAvatarManager.h"

#include "StandardTankAvatar.h"


// new avatar stuff, just return a standard tank for now;
PlayerAvatar* getPlayerAvatar(int playerID, const fvec3& pos,
                                            const fvec3& forward)
{
  return new StandardTankAvatar(playerID, pos, forward);
}


void freePlayerAvatar(PlayerAvatar* avatar)
{
  delete(avatar);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
