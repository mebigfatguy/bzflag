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

/* interface header */
#include "RandomSpawnPolicy.h"

/* common headers */
#include "GameKeeper.h"
#include "common/BzTime.h"
#include "game/PlayerInfo.h"
#include "common/StateDatabase.h"
#include "game/BZDBCache.h"

/* server headers */
#include "bzfs.h"
#include "DropGeometry.h"


RandomSpawnPolicy::RandomSpawnPolicy() {
}

RandomSpawnPolicy::~RandomSpawnPolicy() {
}

void RandomSpawnPolicy::getPosition(fvec3& pos, int playerId,
                                    bool onGroundOnly, bool /*notNearEdges*/) {
  /* the player is coming to life, depending on who they are an what
   * style map/configuration is being played determines how they will
   * spawn.
   */

  GameKeeper::Player* playerData
    = GameKeeper::Player::getPlayerByIndex(playerId);
  if (!playerData) {
    return;
  }

  const PlayerInfo& pi = playerData->player;
  TeamColor t = pi.getTeam();

  if (!BZDB.isTrue("freeCtfSpawns") &&
      playerData->player.shouldRestartAtBase() &&
      (t >= RedTeam) && (t <= PurpleTeam) &&
      (bases.find(t) != bases.end())) {

    /* if the player needs to spawn on a base, select a random
     * position on one of their team's available bases.
     */

    TeamBases& teamBases = bases[t];
    const TeamBase& base = teamBases.getRandomBase();
    base.getRandomPosition(pos);
    playerData->player.setRestartOnBase(false);

  }
  else {
    /* *** "random" spawn position selection occurs below here. ***
     *
     * Basic idea is to just pick a purely random valid point and just
     * go with it.
     */

    const float size = BZDBCache::worldSize;
    const float maxHeight = world->getMaxWorldHeight();

    // keep track of how much time we spend searching for a location
    BzTime start = BzTime::getCurrent();

    int tries = 0;
    bool foundspot = false;
    while (!foundspot) {
      if (!world->getPlayerSpawnPoint(&pi, pos)) {
        pos.x = ((float)bzfrand() - 0.5f) * size;
        pos.y = ((float)bzfrand() - 0.5f) * size;
        pos.z = onGroundOnly ? 0.0f : ((float)bzfrand() * maxHeight);
      }
      tries++;

      const float waterLevel = world->getWaterLevel();
      float minZ = 0.0f;
      if (waterLevel > minZ) {
        minZ = waterLevel;
      }
      float maxZ = maxHeight;
      if (onGroundOnly) {
        maxZ = 0.0f;
      }

      if (DropGeometry::dropPlayer(pos, minZ, maxZ)) {
        foundspot = true;
      }

      // check every now and then if we have already used up 10ms of time
      if (tries >= 50) {
        tries = 0;
        if (BzTime::getCurrent() - start > BZDB.eval("_spawnMaxCompTime")) {
          //Just drop the sucka in, and pray
          debugf(1, "Warning: RandomSpawnPolicy ran out of time, just dropping the sucker in\n");
          break;
        }
      }
    }
  }
}

void RandomSpawnPolicy::getAzimuth(float& azimuth) {
  azimuth = (float)(bzfrand() * 2.0 * M_PI);
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
