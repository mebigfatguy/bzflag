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

#include "Plan.h"
#include "BZDBCache.h"
#include "playing.h"
#include "LocalPlayer.h"
#include "WorldPlayer.h"
#include "Roster.h"
#include "TargetingUtils.h"
#include "World.h"

/**
 * Plan
 */
Plan::Plan(float planDuration) {
  planExpiration = BzTime::getCurrent();
  planExpiration += planDuration;
}


Plan::~Plan() {
}


bool Plan::isValid() {
  BzTime now = BzTime();
  float delta = float(now - planExpiration);
  return (delta < 0.0f);
}

void Plan::execute(float&, float&) {
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  World* world = World::getWorld();
  if (!myTank || !world) {
    return;
  }
  fvec3 pos = myTank->getPosition();
  if (pos.z < 0.0f) {
    pos.z = 0.01f;
  }
  float myAzimuth = myTank->getAngle();

  fvec3 dir(cosf(myAzimuth), sinf(myAzimuth), 0.0f);
  pos[2] += myTank->getMuzzleHeight();
  Ray tankRay(pos, dir);
  pos[2] -= myTank->getMuzzleHeight();

  if (myTank->getFlagType() == Flags::ShockWave) {
    BzTime now = BzTime::getTick();
    if (now - lastShot >= (1.0f / world->getMaxShots())) {
      bool hasSWTarget = false;
      for (int t = 0; t < curMaxPlayers; t++) {
        if (t != myTank->getId() && remotePlayers[t] &&
            remotePlayers[t]->isAlive() && !remotePlayers[t]->isPaused() &&
            !remotePlayers[t]->isNotResponding()) {

          const fvec3& tp = remotePlayers[t]->getPosition();
          const fvec3& tv = remotePlayers[t]->getVelocity();
          // toss in some lag adjustment/future prediction - 300 millis
          fvec3 enemyPos = tp + (0.3f * tv);
          if (enemyPos.z < 0.0f) {
            enemyPos.z = 0.0f;
          }

          float dist = TargetingUtils::getTargetDistance(pos, enemyPos);
          if (dist <= BZDB.eval(BZDBNAMES.SHOCKOUTRADIUS)) {
            if (!myTank->validTeamTarget(remotePlayers[t])) {
              hasSWTarget = false;
              t = curMaxPlayers;
            }
            else {
              hasSWTarget = true;
            }
          }
        }
      }
      if (hasSWTarget) {
        myTank->fireShot();
        lastShot = BzTime::getTick();
      }
    }
  }
  else {
    BzTime now = BzTime::getTick();
    if (now - lastShot >= (1.0f / world->getMaxShots())) {

      float errorLimit = world->getMaxShots() * BZDB.eval(BZDBNAMES.LOCKONANGLE) / 8.0f;
      float closeErrorLimit = errorLimit * 2.0f;

      for (int t = 0; t < curMaxPlayers; t++) {
        if (t != myTank->getId() && remotePlayers[t] &&
            remotePlayers[t]->isAlive() && !remotePlayers[t]->isPaused() &&
            !remotePlayers[t]->isNotResponding() &&
            myTank->validTeamTarget(remotePlayers[t])) {

          if (remotePlayers[t]->isPhantomZoned() && !myTank->isPhantomZoned()
              && (myTank->getFlagType() != Flags::SuperBullet)) {
            continue;
          }

          const fvec3& tp = remotePlayers[t]->getPosition();
          const fvec3& tv = remotePlayers[t]->getVelocity();
          // toss in some lag adjustment/future prediction - 300 millis
          fvec3 enemyPos = tp + (0.3f * tv);
          if (enemyPos.z < 0.0f) {
            enemyPos.z = 0.0f;
          }

          float dist = TargetingUtils::getTargetDistance(pos, enemyPos);

          if ((myTank->getFlagType() == Flags::GuidedMissile) ||
              (fabs(pos[2] - enemyPos[2]) < 2.0f * BZDBCache::tankHeight)) {

            float targetDiff = TargetingUtils::getTargetAngleDifference(pos, myAzimuth, enemyPos);
            if ((targetDiff < errorLimit) ||
                ((dist < (2.0f * BZDB.eval(BZDBNAMES.SHOTSPEED))) &&
                 (targetDiff < closeErrorLimit))) {
              bool isTargetObscured;
              if (myTank->getFlagType() != Flags::SuperBullet) {
                isTargetObscured = TargetingUtils::isLocationObscured(pos, enemyPos);
              }
              else {
                isTargetObscured = false;
              }

              if (!isTargetObscured) {
                myTank->fireShot();
                lastShot = now;
                t = curMaxPlayers;
              }
            }
          }
        }
      }
    }
  }
}

bool Plan::avoidBullet(float& rotation, float& speed) {
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  World* world = World::getWorld();
  if (!myTank || !world) {
    return false;
  }
  const fvec3& pos = myTank->getPosition();

  if ((myTank->getFlagType() == Flags::Narrow) ||
      (myTank->getFlagType() == Flags::Burrow)) {
    return false;  // take our chances
  }

  float minDistance;
  ShotPath* shot = findWorstBullet(minDistance);

  if ((shot == NULL) || (minDistance > 100.0f)) {
    return false;
  }

  const fvec3& shotPos = shot->getPosition();
  const fvec3& shotVel = shot->getVelocity();
  const float  shotAngle = atan2f(shotVel.y, shotVel.x);
  const fvec2  shotUnitVec(cosf(shotAngle), sinf(shotAngle));
  const fvec2  trueVec = (pos.xy() - shotPos.xy()) / minDistance;
  const float  dotProd = fvec2::dot(trueVec, shotUnitVec);

  if ((myTank->canJump()) &&
      (minDistance < (std::max(dotProd, 0.5f) * BZDBCache::tankLength * 2.25f))) {
    myTank->setJump();
    return (myTank->getFlagType() != Flags::Wings);
  }
  else if (dotProd > 0.96f) {
    speed = 1.0;
    float myAzimuth = myTank->getAngle();
    float rotation1 = TargetingUtils::normalizeAngle((float)((shotAngle + M_PI / 2.0) - myAzimuth));

    float rotation2 = TargetingUtils::normalizeAngle((float)((shotAngle - M_PI / 2.0) - myAzimuth));

    float zCross = shotUnitVec[0] * trueVec[1] - shotUnitVec[1] * trueVec[0];

    if (zCross > 0.0f) { //if i am to the left of the shot from shooter pov
      rotation = rotation1;
      if (fabs(rotation1) < fabs(rotation2)) {
        speed = 1.0f;
      }
      else if (dotProd > 0.98f) {
        speed = -0.5f;
      }
      else {
        speed = 0.5f;
      }
    }
    else {
      rotation = rotation2;
      if (fabs(rotation2) < fabs(rotation1)) {
        speed = 1.0f;
      }
      else if (dotProd > 0.98f) {
        speed = -0.5f;
      }
      else {
        speed = 0.5f;
      }
    }

    return true;
  }
  return false;
}

ShotPath* Plan::findWorstBullet(float& minDistance) {
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  World* world = World::getWorld();
  if (!myTank || !world) {
    return NULL;
  }
  const fvec3& pos = myTank->getPosition();
  ShotPath* minPath = NULL;

  minDistance = Infinity;
  for (int t = 0; t < curMaxPlayers; t++) {
    if (t == myTank->getId() || !remotePlayers[t]) {
      continue;
    }

    const int maxShots = remotePlayers[t]->getMaxShots();
    for (int s = 0; s < maxShots; s++) {
      ShotPath* shot = remotePlayers[t]->getShot(s);
      if (!shot || shot->isExpired()) {
        continue;
      }

      if ((shot->getShotType() == InvisibleShot ||
           shot->getShotType() == CloakedShot) &&
          (myTank->getFlagType() != Flags::Seer)) {
        continue;  //Theoretically Roger could triangulate the sound
      }
      if (remotePlayers[t]->isPhantomZoned() && !myTank->isPhantomZoned()) {
        continue;
      }
      if ((shot->getShotType() == LaserShot) &&
          (myTank->getFlagType() == Flags::Cloaking)) {
        continue;  //cloaked tanks can't die from lasers
      }

      const fvec3& shotPos = shot->getPosition();
      if ((fabs(shotPos[2] - pos[2]) > BZDBCache::tankHeight) &&
          (shot->getShotType() != GMShot)) {
        continue;
      }

      const float dist = TargetingUtils::getTargetDistance(pos, shotPos);
      if (dist < minDistance) {
        const fvec3& shotVel = shot->getVelocity();
        float shotAngle = atan2f(shotVel[1], shotVel[0]);
        float shotUnitVec[2] = {cosf(shotAngle), sinf(shotAngle)};

        float trueVec[2] = { (pos[0] - shotPos[0]) / dist, (pos[1] - shotPos[1]) / dist };
        float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

        if (dotProd <= 0.1f) { //pretty wide angle, evasive actions prolly aren't gonna work
          continue;
        }

        minDistance = dist;
        minPath = shot;
      }
    }
  }

  float oldDistance = minDistance;
  WorldPlayer* wp = world->getWorldWeapons();
  for (int w = 0; w < wp->getMaxShots(); w++) {
    ShotPath* shot = wp->getShot(w);
    if (!shot || shot->isExpired()) {
      continue;
    }

    if ((shot->getShotType() == InvisibleShot || shot->getShotType() == CloakedShot) &&
        (myTank->getFlagType() != Flags::Seer)) {
      continue; //Theoretically Roger could triangulate the sound
    }
    if (shot->getShotType() == LaserShot &&
        myTank->getFlagType() == Flags::Cloaking) {
      continue; //cloaked tanks can't die from lasers
    }

    const fvec3& shotPos = shot->getPosition();
    if ((fabs(shotPos[2] - pos[2]) > BZDBCache::tankHeight) && (shot->getShotType() != GMShot)) {
      continue;
    }

    const float dist = TargetingUtils::getTargetDistance(pos, shotPos);
    if (dist < minDistance) {
      const fvec3& shotVel = shot->getVelocity();
      float shotAngle = atan2f(shotVel[1], shotVel[0]);
      float shotUnitVec[2] = {cosf(shotAngle), sinf(shotAngle)};

      float trueVec[2] = { (pos[0] - shotPos[0]) / dist,
                           (pos[1] - shotPos[1]) / dist
                         };
      float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

      if (dotProd <= 0.1f) { //pretty wide angle, evasive actions prolly aren't gonna work
        continue;
      }

      minDistance = dist;
      minPath = shot;
    }
  }
  if (oldDistance < minDistance) {
    minDistance = oldDistance;  //pick the closer bullet
  }
  return minPath;
}


/**
 * PlanStack
 */

PlanStack::PlanStack() {
  Plan* pPlan = new TopLevelPlan();
  plans.push(pPlan);
}

PlanStack::~PlanStack() {
  while (plans.size() > 0) {
    Plan* pPlan = plans.top();
    delete pPlan;
    plans.pop();
  }
}

void PlanStack::execute(float& rotation, float& speed) {
  if (Plan::avoidBullet(rotation, speed)) {
    return;
  }

  Plan* pPlan = NULL;

  while (plans.size() > 0) {
    pPlan = plans.top();
    if (!pPlan->isValid()) {
      delete pPlan;
      plans.pop();
    }
  }

  while (pPlan->usesSubPlan()) {
    pPlan = pPlan->createSubPlan();
    plans.push(pPlan);
  }

  pPlan->execute(rotation, speed);
}

/**
 * TopLevelPlan
 */

TopLevelPlan::TopLevelPlan()
  : Plan(0) {
}

bool TopLevelPlan::isValid() {
  //always valid
  return true;
}

bool TopLevelPlan::usesSubPlan() {
  return true;
}

Plan* TopLevelPlan::createSubPlan() {
  //TODO: Pick a plan
  return NULL;
}


/**
 * GotoPointPlan
 */

GotoPointPlan::GotoPointPlan(const fvec3& pt)
  : Plan(20.0f)
  , gotoPt(pt) {
}

bool GotoPointPlan::usesSubPlan() {
  return false;
}

Plan* GotoPointPlan::createSubPlan() {
  return NULL;
}

void GotoPointPlan::execute(float& rotation, float& speed) {
  //TODO: goto point, then

  Plan::execute(rotation, speed);
}

/**
 * WeavePlan
 */

WeavePlan::WeavePlan(int pID, bool right)
  : Plan(10.0) {
  playerID = pID;
  weaveRight = right;
}

bool WeavePlan::isValid() {
  Player* pPlayer = lookupPlayer(playerID);
  if (pPlayer == NULL) {
    return false;
  }

  if (!pPlayer->isAlive()) {
    return false;
  }

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  const fvec3& pVel = myTank->getVelocity();
  if ((pVel[0] == 0.0f) && (pVel[1] == 0.0f) && (pVel[2] == 0.0f)) {
    return false;
  }

  return true;
}

bool WeavePlan::usesSubPlan() {
  return false;
}

Plan* WeavePlan::createSubPlan() {
  return NULL;
}

void WeavePlan::execute(float& rotation, float& speed) {
  //TODO: weave, then

  Plan::execute(rotation, speed);
}

/**
 * HuntPlayerPlan
 */

HuntPlayerPlan::HuntPlayerPlan()
  : Plan(300.0f) {
  //Pick a player ID to hunt
  playerID = 0;
}

bool HuntPlayerPlan::isValid() {
  if (!Plan::isValid()) {
    return false;
  }

  Player* pPlayer = lookupPlayer(playerID);
  if (pPlayer == NULL) {
    return false;
  }

  if (!pPlayer->isAlive()) {
    return false;
  }

  LocalPlayer* myTank = LocalPlayer::getMyTank();
  if (pPlayer->getTeam() == myTank->getTeam()) {
    return false;
  }

  return true;
}

bool HuntPlayerPlan::usesSubPlan() {
  return true;
}

Plan* HuntPlayerPlan::createSubPlan() {
  Player* pPlayer = lookupPlayer(playerID);
  LocalPlayer* myTank = LocalPlayer::getMyTank();
  bool isObscured = TargetingUtils::isLocationObscured(myTank->getPosition(),
                                                       pPlayer->getPosition());
  if (isObscured) {
    fvec3 pt;
    // fill in pt with a open spot to go to
    return new GotoPointPlan(pt);
  }
  else {
    return new WeavePlan(playerID, bzfrand() > 0.5f);
  }
}


/**
 * HuntTeamFlagPlan
 */

HuntTeamFlagPlan::HuntTeamFlagPlan()
  : Plan(300.0f) {
}

bool HuntTeamFlagPlan::isValid() {
  return false;
}

bool HuntTeamFlagPlan::usesSubPlan() {
  return true;
}

Plan* HuntTeamFlagPlan::createSubPlan() {
  return NULL;
}

/**
 * CaptureFlagPlan
 */

CaptureFlagPlan::CaptureFlagPlan()
  : Plan(1200.0f) {
}

bool CaptureFlagPlan::isValid() {
  return false;
}

bool CaptureFlagPlan::usesSubPlan() {
  return true;
}

Plan* CaptureFlagPlan::createSubPlan() {
  return NULL;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
