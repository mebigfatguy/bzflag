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

#include <stack>
#include "TimeKeeper.h"
#include "Roster.h"
#include "TargetingUtils.h"
#include "World.h"
#include "WorldPlayer.h"
#include "BZDBCache.h"
#include "ServerLink.h"
#include "playing.h"

typedef std::map<FlagType*, std::pair<int,int> > FlagSuccessMap;

static FlagSuccessMap flagSuccess;
static int			  totalSum = 0;
static int			  totalCnt = 0;

/**
 * Roger will follow a plan. These plans are stored on a plan stack
 * allowing for short medium and long range plans to cooperate to 
 * produce a better playing bot.
 */
class Plan
{
public:
	Plan( float planDuration );
	/**
	 * checks to make sure this plan is still valid. The default
	 * implementation just checks for an expiration time, to keep
	 * roger from bullheadedly trying something that just aint
	 * gonna work. You can override this method for specific plans
	 */
	virtual bool isValid();

	/**
	 * determines if this plan is a longer range plan that can
	 * only be accomplished with smaller steps. Hunting a player
	 * might consist of avoiding a building, or weaving, for instance.
	 * if this method returns true, than this plan is just a coordinator
	 * of sub plans.
     */
	virtual bool usesSubPlan() = 0;

	/**
	 * creates a sub plan to be placed on the stack to accomplish
	 * this plan's goals.
	 */
	virtual Plan *createSubPlan() = 0;

	/**
	 * execute the plan. set the rotation and speed
	 *
	 */
	virtual void execute( float &rotation, float &velocity ) = 0;

private:
	TimeKeeper planExpiration;
};

static std::stack<std::vector<Plan *> > planStack;

void teachAutoPilot(FlagType *type, int adjust)
{
  if (type == Flags::Null)
    return;

  FlagSuccessMap::iterator it = flagSuccess.find(type);
  if (it != flagSuccess.end()) {
    std::pair<int,int> &pr = it->second;
    pr.first += adjust;
    pr.second++;
  } else {
    flagSuccess[type] = std::pair<int,int>(adjust,1);
  }
  totalSum += adjust;
  totalCnt++;
}

bool isFlagUseful(FlagType *type)
{
  if (type == Flags::Null)
    return false;

  FlagSuccessMap::iterator it = flagSuccess.find(type);
  float flagValue;
  if (it != flagSuccess.end()) {
    std::pair<int,int> &pr = it->second;
    if (pr.second == 0)
      flagValue = 0.0f;
    else
      flagValue = (float)pr.first / (float)pr.second;
  } else {
    return true;
  }

  float avg;
  if (totalCnt == 0)
    avg = 0.0f;
  else
    avg = (float)totalSum / (float)totalCnt;
  return ((float)flagValue) >= avg;
}

float normalizeAngle(float ang)
{
  if (ang < -1.0f * M_PI) ang += (float)(2.0 * M_PI);
  if (ang > 1.0f * M_PI) ang -= (float)(2.0 * M_PI);
  return ang;
}

ShotPath *findWorstBullet(float &minDistance)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();
  ShotPath *minPath = NULL;

  minDistance = Infinity;
  for (int t = 0; t < curMaxPlayers; t++) {
    if (t == myTank->getId() || !player[t])
      continue;

    const int maxShots = player[t]->getMaxShots();
    for (int s = 0; s < maxShots; s++) {
      ShotPath* shot = player[t]->getShot(s);
      if (!shot || shot->isExpired())
	continue;

      if ((shot->getFlag() == Flags::InvisibleBullet) &&
	  (myTank->getFlag() != Flags::Seer))
	continue; //Theoretically Roger could triangulate the sound
      if (player[t]->isPhantomZoned() && !myTank->isPhantomZoned())
	continue;
      if ((shot->getFlag() == Flags::Laser) &&
	  (myTank->getFlag() == Flags::Cloaking))
	continue; //cloaked tanks can't die from lasers

      const float* shotPos = shot->getPosition();
      if ((fabs(shotPos[2] - pos[2]) > BZDBCache::tankHeight) &&
	  (shot->getFlag() != Flags::GuidedMissile))
	continue;

      const float dist = TargetingUtils::getTargetDistance(pos, shotPos);
      if (dist < minDistance) {
	const float *shotVel = shot->getVelocity();
	float shotAngle = atan2f(shotVel[1], shotVel[0]);
	float shotUnitVec[2] = {cos(shotAngle), sin(shotAngle)};

	float trueVec[2] = { (pos[0] - shotPos[0]) / dist, (pos[1] - shotPos[1]) / dist };
	float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

	if (dotProd <= 0.1f) //pretty wide angle, evasive actions prolly aren't gonna work
	  continue;

	minDistance = dist;
	minPath = shot;
      }
    }
  }
  float oldDistance = minDistance;
  WorldPlayer *wp = World::getWorld()->getWorldWeapons();
  for (int w = 0; w < wp->getMaxShots(); w++) {
    ShotPath* shot = wp->getShot(w);
    if (!shot || shot->isExpired())
      continue;

    if (shot->getFlag() == Flags::InvisibleBullet && myTank->getFlag() != Flags::Seer)
      continue; //Theoretically Roger could triangulate the sound
    if (shot->getFlag() == Flags::Laser && myTank->getFlag() == Flags::Cloaking)
      continue; //cloaked tanks can't die from lasers

    const float* shotPos = shot->getPosition();
    if ((fabs(shotPos[2] - pos[2]) > BZDBCache::tankHeight) && (shot->getFlag() != Flags::GuidedMissile))
      continue;

    const float dist = TargetingUtils::getTargetDistance( pos, shotPos );
    if (dist < minDistance) {
      const float *shotVel = shot->getVelocity();
      float shotAngle = atan2f(shotVel[1], shotVel[0]);
      float shotUnitVec[2] = {cos(shotAngle), sin(shotAngle)};

      float trueVec[2] = { (pos[0] - shotPos[0]) / dist, (pos[1] - shotPos[1]) / dist };
      float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

      if (dotProd <= 0.1f) //pretty wide angle, evasive actions prolly aren't gonna work
	continue;

      minDistance = dist;
      minPath = shot;
    }
  }
  if (oldDistance < minDistance)
    minDistance = oldDistance; //pick the closer bullet
  return minPath;
}

bool	avoidDeathFall(float &/*rotation*/, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  float pos1[3], pos2[3];
  memcpy(pos1, myTank->getPosition(), sizeof(pos1));
  memcpy(pos2, pos1, sizeof(pos1));
  pos1[2] += 10.0f * BZDBCache::tankHeight;
  float azimuth = myTank->getAngle();
  if (speed < 0.0f)
	  azimuth = fmodf(float(azimuth + M_PI), float(2.0 * M_PI));
  else
	  azimuth = fmodf(float(azimuth), float(2.0 * M_PI));

  pos2[0] += 8.0f * BZDBCache::tankHeight * cosf(azimuth);
  pos2[1] += 8.0f * BZDBCache::tankHeight * sinf(azimuth);
  pos2[2] += 0.01f;

  float collisionPt[3];
  if (TargetingUtils::getFirstCollisionPoint( pos1, pos2, collisionPt )) {
	  if (collisionPt[2] < 0.0f)
		  collisionPt[2] = 0.0f;
	  if (collisionPt[2] < World::getWorld()->getWaterLevel()) {
		  speed = 0.0f;
		  return true;
	  }
  } else if (collisionPt[2] < (pos2[2] - 1.0f)) {
	  speed *= 0.5f;
  }

  return false;
}

bool	avoidBullet(float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();

  if ((myTank->getFlag() == Flags::Narrow) || (myTank->getFlag() == Flags::Burrow))
    return false; // take our chances

  float minDistance;
  ShotPath *shot = findWorstBullet(minDistance);

  if ((shot == NULL) || (minDistance > 100.0f))
    return false;

  const float *shotPos = shot->getPosition();
  const float *shotVel = shot->getVelocity();
  float shotAngle = atan2f(shotVel[1],shotVel[0]);
  float shotUnitVec[2] = {cos(shotAngle), sin(shotAngle)};

  float trueVec[2] = {(pos[0]-shotPos[0])/minDistance,(pos[1]-shotPos[1])/minDistance};
  float dotProd = trueVec[0]*shotUnitVec[0]+trueVec[1]*shotUnitVec[1];

#ifdef _MSC_VER
  if (((World::getWorld()->allowJumping() || (myTank->getFlag()) == Flags::Jumping
   || (myTank->getFlag()) == Flags::Wings))
   && (minDistance < (max(dotProd,0.5f) * BZDBCache::tankLength * 2.25f))
   && (myTank->getFlag() != Flags::NoJumping)) {
#else
  if (((World::getWorld()->allowJumping() || (myTank->getFlag()) == Flags::Jumping
   || (myTank->getFlag()) == Flags::Wings))
   && (minDistance < (std::max(dotProd,0.5f) * BZDBCache::tankLength * 2.25f))
   && (myTank->getFlag() != Flags::NoJumping)) {
#endif
    myTank->setJump();
    return (myTank->getFlag() != Flags::Wings);
  } else if (dotProd > 0.96f) {
    speed = 1.0;
    float myAzimuth = myTank->getAngle();
    float rotation1 = normalizeAngle((float)((shotAngle + M_PI/2.0) - myAzimuth));

    float rotation2 = normalizeAngle((float)((shotAngle - M_PI/2.0) - myAzimuth));

    float zCross = shotUnitVec[0]*trueVec[1] - shotUnitVec[1]*trueVec[0];

    if (zCross > 0.0f) { //if i am to the left of the shot from shooter pov
      rotation = rotation1;
      if (fabs(rotation1) < fabs(rotation2))
	speed = 1.0f;
      else if (dotProd > 0.98f)
	speed = -0.5f;
      else
	speed = 0.5f;
    } else {
      rotation = rotation2;
      if (fabs(rotation2) < fabs(rotation1))
	speed = 1.0f;
      else if (dotProd > 0.98f)
	speed = -0.5f;
      else
	speed = 0.5f;
    }
    return true;
  }
  return false;
}

bool	stuckOnWall(float &rotation, float &speed)
{
  static TimeKeeper lastStuckTime;
  static float stuckRot = 0.0f, stuckSpeed = 0.0f;

  float stuckPeriod = TimeKeeper::getCurrent() - lastStuckTime;
  if (stuckPeriod < 0.5f) {
    rotation = stuckRot;
    speed = stuckSpeed;
    return true;
  } else if (stuckPeriod < 1.0f) {
    rotation = stuckRot;
    speed = 1.0;
    return true;
  }

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();
  float myAzimuth = myTank->getAngle();

  const bool phased = (myTank->getFlag() == Flags::OscillationOverthruster)
		      || myTank->isPhantomZoned();

  if (!phased && (TargetingUtils::getOpenDistance(pos, myAzimuth) < 5.0f)) {
    lastStuckTime = TimeKeeper::getCurrent();
    if (bzfrand() > 0.8f) {
      // Every once in a while, do something nuts
      speed = (float)(bzfrand() * 1.5f - 0.5f);
      rotation = (float)(bzfrand() * 2.0f - 1.0f);
    } else {
      float leftDistance = TargetingUtils::getOpenDistance( pos, (float)(myAzimuth + (M_PI/4.0)));
      float rightDistance = TargetingUtils::getOpenDistance( pos, (float)(myAzimuth - (M_PI/4.0)));
      if (leftDistance > rightDistance)
	rotation = 1.0f;
      else
	rotation = -1.0f;
      speed = -0.5f;
    }
    stuckRot = rotation;
    stuckSpeed = speed;
    return true;
  }
  return false;
}

RemotePlayer *findBestTarget()
{
  RemotePlayer *target = NULL;
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();
  float myAzimuth = myTank->getAngle();
  float distance = Infinity;

  for (int t = 0; t < curMaxPlayers; t++) {
    if ((t != myTank->getId())
    &&  (player[t])
    &&  (player[t]->isAlive())
    &&  (!player[t]->isPaused())
    &&  (!player[t]->isNotResponding())
    &&  (myTank->validTeamTarget(player[t]))) {

      if (player[t]->isPhantomZoned() && !myTank->isPhantomZoned()
	  && (myTank->getFlag() != Flags::ShockWave)
	  && (myTank->getFlag() != Flags::SuperBullet))
	continue;

      if ((player[t]->getFlag() == Flags::Cloaking) &&
	  (myTank->getFlag() == Flags::Laser))
	continue;

      //perform a draft that has us chase the proposed opponent if they have our flag
      if (World::getWorld()->allowTeamFlags() &&
	  (myTank->getTeam() == RedTeam && player[t]->getFlag() == Flags::RedTeam) ||
	  (myTank->getTeam() == GreenTeam && player[t]->getFlag() == Flags::GreenTeam) ||
	  (myTank->getTeam() == BlueTeam && player[t]->getFlag() == Flags::BlueTeam) ||
				(myTank->getTeam() == PurpleTeam && player[t]->getFlag() == Flags::PurpleTeam)) {
	target = player[t];
	break;
      }

      float d = TargetingUtils::getTargetDistance(pos, player[t]->getPosition());
      bool isObscured = TargetingUtils::isLocationObscured( pos, player[t]->getPosition());
      if (isObscured) //demote the priority of obscured enemies
	d *= 1.25f;

      if (d < distance) {
	if ((player[t]->getFlag() != Flags::Stealth)
	||  (myTank->getFlag() == Flags::Seer)
	||  ((!isObscured) &&
	     (TargetingUtils::getTargetAngleDifference(pos, myAzimuth, player[t]->getPosition()) <= 30.0f))) {
	  target = player[t];
	  distance = d;
	}
      }
    }
  }

  return target;
}

bool chasePlayer(float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  const float *pos = myTank->getPosition();

  RemotePlayer *player = findBestTarget();
  if (player == NULL)
    return false;

  myTank->setTarget(player);

  const float *targetPos = player->getPosition();
  float distance = TargetingUtils::getTargetDistance(pos, targetPos);
  if (distance > 250.0f)
    return false;

  const float *tp = player->getPosition();
  float enemyPos[3];
  //toss in some lag adjustment/future prediction - 300 millis
  memcpy(enemyPos,tp,sizeof(enemyPos));
  const float *tv = player->getVelocity();
  enemyPos[0] += 0.3f * tv[0];
  enemyPos[1] += 0.3f * tv[1];
  enemyPos[2] += 0.3f * tv[2];
  if (enemyPos[2] < 0.0f) //Roger doesn't worry about burrow
    enemyPos[2] = 0.0;

  float myAzimuth = myTank->getAngle();
  float enemyAzimuth = TargetingUtils::getTargetAzimuth( pos, tp );
  rotation = TargetingUtils::getTargetRotation( myAzimuth, enemyAzimuth );

  //If we are driving relatively towards our target and a building pops up jump over it
  if (fabs(rotation) < BZDB.eval(StateDatabase::BZDB_LOCKONANGLE)) {
    const Obstacle *building = NULL;
    float d = distance - 5.0f; //Make sure building is REALLY in front of player (-5)

    float dir[3] = {cosf(myAzimuth), sinf(myAzimuth), 0.0f};
    Ray tankRay(pos, dir);

    building = ShotStrategy::getFirstBuilding(tankRay, -0.5f, d);
    if (building && !myTank->isPhantomZoned() &&
	(myTank->getFlag() != Flags::OscillationOverthruster)) {
      //If roger can drive around it, just do that

      float leftDistance = TargetingUtils::getOpenDistance( pos, (float)(myAzimuth + (M_PI/6.0)));
      if (leftDistance > (2.0f * d)) {
	speed = 0.5f;
	rotation = -0.5f;
	return true;
      }
      float rightDistance = TargetingUtils::getOpenDistance( pos, (float)(myAzimuth - (M_PI/6.0)));
      if (rightDistance > (2.0f * d)) {
	speed = 0.5f;
	rotation = 0.5f;
	return true;
      }

      //Never did good in math, he should really see if he can reach the building
      //based on jumpvel and gravity, but settles for assuming 20-50 is a good range
      if ((d > 20.0f) && (d < 50.0f) &&
	  (building->getType() == BoxBuilding::getClassName())) {
	float jumpVel = BZDB.eval(StateDatabase::BZDB_JUMPVELOCITY);
	float maxJump = (jumpVel * jumpVel) / (2 * -BZDBCache::gravity);

	if (((building->getPosition()[2] - pos[2] + building->getHeight())) < maxJump) {
	  speed = d / 50.0f;
	  myTank->setJump();
	  return true;
	}
      }
    }
  }

  // weave towards the player
  const Player *target = myTank->getTarget();
  if ((distance > (BZDB.eval(StateDatabase::BZDB_SHOTSPEED) /2.0f))
      ||  (myTank->getFiringStatus() != LocalPlayer::Ready)) {
    float enemyUnitVec[2] = { cos(enemyAzimuth), sin(enemyAzimuth) };
    float myUnitVec[2] = { cos(myAzimuth), sin(myAzimuth) };
    float dotProd = (myUnitVec[0] * enemyUnitVec[0] + myUnitVec[1] * enemyUnitVec[1]);
    if (dotProd < 0.866f) {
      //if target is more than 30 degrees away, turn as fast as you can
      rotation *= (float)M_PI / (2.0f * fabs(rotation));
      speed = dotProd; //go forward inverse rel to how much you need to turn
    } else {
      int period = int(TimeKeeper::getTick().getSeconds());
      float absBias = (float)(M_PI/20.0 * (distance / 100.0));
      float bias = ((period % 4) < 2) ? absBias : -absBias;
      rotation += bias;
      rotation = normalizeAngle(rotation);
      speed = 1.0;
    }
  } else if (target->getFlag() != Flags::Burrow) {
    speed = -0.5f;
    rotation *= (float)(M_PI / (2.0 * fabs(rotation)));
  }

  return true;
}

bool lookForFlag(float &rotation, float &speed)
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  float pos[3];

  memcpy( pos, myTank->getPosition(), sizeof( pos ));
  if (pos[2] < 0.0f)
    pos[2] = 0.0f;
  World *world = World::getWorld();
  int closestFlag = -1;

  if ((myTank->getFlag() != Flags::Null)
      && (isFlagUseful(myTank->getFlag())))
    return false;

  float minDist = Infinity;
	int teamFlag = -1;
  for (int i = 0; i < numFlags; i++) {
    if ((world->getFlag(i).type == Flags::Null)
     || (world->getFlag(i).status != FlagOnGround))
      continue;

    if (world->getFlag(i).type->flagTeam != NoTeam)
      teamFlag = i;
    const float* fpos = world->getFlag(i).position;
    if (fpos[2] == pos[2]) {
      float dist = TargetingUtils::getTargetDistance(pos, fpos);
      bool isTargetObscured = TargetingUtils::isLocationObscured(pos, fpos);
      if (isTargetObscured)
	dist *= 1.25f;

      if ((dist < 200.0f) && (dist < minDist)) {
	minDist = dist;
	closestFlag = i;
      }
    }
  }

  if (teamFlag != -1 && (minDist < 10.0f || closestFlag == -1))
    closestFlag = teamFlag; //FIXME: should a team flag be more significant than a closer flag?
  if (closestFlag != -1) {
    if (minDist < 10.0f || teamFlag != -1) {
      if (myTank->getFlag() != Flags::Null) {
	serverLink->sendDropFlag(myTank->getPosition());
	handleFlagDropped(myTank);
      }
    }

    const float *fpos = world->getFlag(closestFlag).position;
    float myAzimuth = myTank->getAngle();
    float flagAzimuth = TargetingUtils::getTargetAzimuth(pos, fpos);
    rotation = TargetingUtils::getTargetRotation(myAzimuth, flagAzimuth);
    speed = (float)(M_PI/2.0 - fabs(rotation));
    return true;
  }

  return false;
}

bool navigate(float &rotation, float &speed)
{
  static TimeKeeper lastNavChange;
  static float navRot = 0.0f, navSpeed = 0.0f;

  if ((TimeKeeper::getCurrent() - lastNavChange) < 1.0f) {
    rotation = navRot;
    speed = navSpeed;
    return true;
  }

  LocalPlayer *myTank = LocalPlayer::getMyTank();
  float pos[3];

  memcpy(pos, myTank->getPosition(), sizeof(pos));
  if (pos[2] < 0.0f)
    pos[2] = 0.01f;
  float myAzimuth = myTank->getAngle();

  float leftDistance = TargetingUtils::getOpenDistance(pos, (float)(myAzimuth + (M_PI/4.0)));
  float centerDistance = TargetingUtils::getOpenDistance(pos, myAzimuth);
  float rightDistance = TargetingUtils::getOpenDistance(pos, (float)(myAzimuth - (M_PI/4.0)));
  if (leftDistance > rightDistance) {
    if (leftDistance > centerDistance)
      rotation = 0.75f;
    else
      rotation = 0.0f;
  } else {
    if (rightDistance > centerDistance)
      rotation = -0.75f;
    else
      rotation = 0.0f;
  }
  if (myTank->getFlag()->flagTeam != NoTeam) {
    World *world = World::getWorld();
    const float *temp = world->getBase(myTank->getTeam());
    if (temp == NULL) {
      serverLink->sendDropFlag(myTank->getPosition());
      handleFlagDropped(myTank);
    }
    if ((((int) *(world->getBase(myTank->getTeam())) + 2
	  >= (int) *(myTank->getPosition()))
	 || (temp[0] == pos[0] && temp[1] == pos[1])) &&
	myTank->getFlag()->flagTeam == myTank->getTeam()) {
      serverLink->sendDropFlag(myTank->getPosition());
      handleFlagDropped(myTank);
    } else {
      float baseAzimuth = TargetingUtils::getTargetAzimuth(pos, temp);
      rotation = TargetingUtils::getTargetRotation(myAzimuth, baseAzimuth);
      speed = (float)(M_PI/2.0 - fabs(rotation));
    }
  } else {
    speed = 1.0f;
  }
  if (myTank->getLocation() == LocalPlayer::InAir
      && myTank->getFlag() == Flags::Wings)
    myTank->setJump();
  navRot = rotation;
  navSpeed = speed;
  lastNavChange = TimeKeeper::getCurrent();
  return true;
}

bool fireAtTank()
{
  static TimeKeeper lastShot;
  float pos[3];
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  memcpy(pos, myTank->getPosition(), sizeof(pos));
  if (pos[2] < 0.0f)
    pos[2] = 0.01f;
  float myAzimuth = myTank->getAngle();

  float dir[3] = {cosf(myAzimuth), sinf(myAzimuth), 0.0f};
  pos[2] += myTank->getMuzzleHeight();
  Ray tankRay(pos, dir);
  pos[2] -= myTank->getMuzzleHeight();

  if (myTank->getFlag() == Flags::ShockWave) {
    TimeKeeper now = TimeKeeper::getTick();
    if (now - lastShot >= (1.0f / World::getWorld()->getMaxShots())) {
      bool hasSWTarget = false;
      for (int t = 0; t < curMaxPlayers; t++) {
	if (t != myTank->getId() && player[t] &&
	    player[t]->isAlive() && !player[t]->isPaused() &&
	    !player[t]->isNotResponding()) {

	  const float *tp = player[t]->getPosition();
	  float enemyPos[3];
	  //toss in some lag adjustment/future prediction - 300 millis
	  memcpy(enemyPos,tp,sizeof(enemyPos));
	  const float *tv = player[t]->getVelocity();
	  enemyPos[0] += 0.3f * tv[0];
	  enemyPos[1] += 0.3f * tv[1];
	  enemyPos[2] += 0.3f * tv[2];
	  if (enemyPos[2] < 0.0f)
	    enemyPos[2] = 0.0f;
	  float dist = TargetingUtils::getTargetDistance( pos, enemyPos );
	  if (dist <= BZDB.eval(StateDatabase::BZDB_SHOCKOUTRADIUS)) {
	    if (!myTank->validTeamTarget(player[t])) {
	      hasSWTarget = false;
	      t = curMaxPlayers;
	    } else {
	      hasSWTarget = true;
	    }
	  }
	}
      }
      if (hasSWTarget) {
	myTank->fireShot();
	lastShot = TimeKeeper::getTick();
	return true;
      }
    }
  } else {
    TimeKeeper now = TimeKeeper::getTick();
    if (now - lastShot >= (1.0f / World::getWorld()->getMaxShots())) {

      float errorLimit = World::getWorld()->getMaxShots() * BZDB.eval(StateDatabase::BZDB_LOCKONANGLE) / 8.0f;
      float closeErrorLimit = errorLimit * 2.0f;

      for (int t = 0; t < curMaxPlayers; t++) {
	if (t != myTank->getId() && player[t] &&
	    player[t]->isAlive() && !player[t]->isPaused() &&
	    !player[t]->isNotResponding() &&
	    myTank->validTeamTarget(player[t])) {

	  if (player[t]->isPhantomZoned() && !myTank->isPhantomZoned()
	      && (myTank->getFlag() != Flags::SuperBullet))
	    continue;

	  const float *tp = player[t]->getPosition();
	  float enemyPos[3];
	  //toss in some lag adjustment/future prediction - 300 millis
	  memcpy(enemyPos,tp,sizeof(enemyPos));
	  const float *tv = player[t]->getVelocity();
	  enemyPos[0] += 0.3f * tv[0];
	  enemyPos[1] += 0.3f * tv[1];
	  enemyPos[2] += 0.3f * tv[2];
	  if (enemyPos[2] < 0.0f)
	    enemyPos[2] = 0.0f;

	  float dist = TargetingUtils::getTargetDistance( pos, enemyPos );

	  if ((myTank->getFlag() == Flags::GuidedMissile) || (fabs(pos[2] - enemyPos[2]) < 2.0f * BZDBCache::tankHeight)) {

	    float targetDiff = TargetingUtils::getTargetAngleDifference(pos, myAzimuth, enemyPos );
	    if ((targetDiff < errorLimit)
	    ||  ((dist < (2.0f * BZDB.eval(StateDatabase::BZDB_SHOTSPEED))) && (targetDiff < closeErrorLimit))) {
	      bool isTargetObscured;
	      if (myTank->getFlag() != Flags::SuperBullet)
		isTargetObscured = TargetingUtils::isLocationObscured( pos, enemyPos );
	      else
		isTargetObscured = false;

	      if (!isTargetObscured) {
		myTank->fireShot();
		lastShot = now;
		t = curMaxPlayers;
		return true;
	      }
	    }
	  }
	}
      }
    }
  }

  return false;
}

void    dropHardFlags()
{
  LocalPlayer *myTank = LocalPlayer::getMyTank();
  FlagType *type = myTank->getFlag();
  if ((type == Flags::Useless)
  ||  (type == Flags::MachineGun)
  ||  (type == Flags::Identify)
  ||  ((type == Flags::PhantomZone) && !myTank->isFlagActive())) {
    serverLink->sendDropFlag(myTank->getPosition());
    handleFlagDropped(myTank);
  }
}

void	doAutoPilot(float &rotation, float &speed)
{
    dropHardFlags(); //Perhaps we should remove this and let learning do it's work
    if (!avoidBullet(rotation, speed)) {
      if (!stuckOnWall(rotation, speed)) {
        if (!chasePlayer(rotation, speed)) {
          if (!lookForFlag(rotation, speed)) {
            navigate(rotation, speed);
          }
        }
      }
    }

    avoidDeathFall(rotation, speed);

    fireAtTank();
}

Plan::Plan(float planDuration)
{
	planExpiration = TimeKeeper::getCurrent();
	planExpiration += planDuration;
}


bool Plan::isValid()
{
	TimeKeeper now = TimeKeeper();
	float delta = now - planExpiration;
	return (delta < 0.0f);
}

class GotoPointPlan : public Plan
{
public:
	GotoPointPlan(float *pt)
		: Plan(20.0f)
	{
		memcpy( gotoPt, pt, sizeof( gotoPt ));
	}
	
	virtual bool usesSubPlan()
	{
		return false;
	}

	virtual Plan *createSubPlan() 
	{
		return NULL;
	}

	virtual void execute( float &rotation, float &velocity )
	{
	}

private:
	float gotoPt[3];
};

class WeavePlan : public Plan
{
public:
	WeavePlan(int pID, bool right )
		: Plan(10.0)
	{
		playerID = pID;
		weaveRight = right;
	}

	virtual bool isValid()
	{
		LocalPlayer *myTank = LocalPlayer::getMyTank();
		const float *pVel = myTank->getVelocity();
		return (pVel[0] > 0.0f) || (pVel[1] > 0.0f) || (pVel[2] > 0.0f);
	}

	virtual bool usesSubPlan()
	{
		return false;
	}

	virtual Plan* createSubPlan()
	{
		return NULL;
	}

	virtual void execute( float &rotation, float &velocity )
	{
	}

private:
	int playerID;
	bool weaveRight;
};

class HuntPlayerPlan : public Plan
{
public:
	HuntPlayerPlan()
		:Plan(MAXFLOAT)
	{
		//Pick a player ID to hunt
		playerID = 0;
	}

	virtual bool isValid()
	{
		if (!Plan::isValid())
			return false;

		Player *pPlayer = lookupPlayer(playerID);
		if (pPlayer == NULL)
			return false;

		if (!pPlayer->isAlive())
			return false;

		LocalPlayer *myTank = LocalPlayer::getMyTank();
		if (pPlayer->getTeam() == myTank->getTeam())
			return false;

		return true;
	}

	virtual bool usesSubPlan()
	{
		return true;
	}

	virtual Plan *createSubPlan()
	{
		Player *pPlayer = lookupPlayer(playerID);
 		LocalPlayer *myTank = LocalPlayer::getMyTank();
        bool isObscured = TargetingUtils::isLocationObscured( myTank->getPosition(), pPlayer->getPosition());
		if (isObscured) {
			float pt[3];
			// fill in pt with a open spot to go to
			return new GotoPointPlan(pt);
		} else {
			return new WeavePlan(playerID, bzfrand() > 0.5f);
		}
	}

	virtual void execute( float &, float *)
	{
	}

private:
	int playerID;
};
