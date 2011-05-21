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

/*
 *
 */

// interface header
#include "RobotPlayer.h"

// common implementation headers
#include "BZDBCache.h"

// local implementation headers
#include "World.h"
#include "RemotePlayer.h"
#include "Intersect.h"
#include "TargetingUtils.h"


std::vector<BzfRegion*>* RobotPlayer::obstacleList = NULL;


RobotPlayer::RobotPlayer(const PlayerId& _id,
                         const char* _name, ServerLink* _server)
  : LocalPlayer(_id, _name, ComputerPlayer)
  , target(NULL)
  , pathIndex(0)
  , timerForShot(0.0f)
  , drivingForward(true) {
  gettingSound = false;
  server       = _server;
}


// estimate a player's position at now+t, similar to dead reckoning
void RobotPlayer::projectPosition(const Player* targ, const float t, fvec3& pos) const {
  double hisx = targ->getPosition()[0];
  double hisy = targ->getPosition()[1];
  double hisz = targ->getPosition()[2];
  double hisvx = targ->getVelocity()[0];
  double hisvy = targ->getVelocity()[1];
  double hisvz = targ->getVelocity()[2];
  double omega = fabs(targ->getAngularVelocity());
  double sx, sy;

  if ((targ->getStatus() & PlayerState::Falling) || fabs(omega) < 2 * M_PI / 360 * 0.5) {
    sx = t * hisvx;
    sy = t * hisvy;
  }
  else {
    double hisspeed = hypotf(hisvx, hisvy);
    double alfa = omega * t;
    double r = hisspeed / fabs(omega);
    double dx = r * sin(alfa);
    double dy2 = r * (1 - cos(alfa));
    double beta = atan2(dy2, dx) * (targ->getAngularVelocity() > 0 ? 1 : -1);
    double gamma = atan2(hisvy, hisvx);
    double rho = gamma + beta;
    sx = hisspeed * t * cos(rho);
    sy = hisspeed * t * sin(rho);
  }
  pos.x = (float)hisx + (float)sx;
  pos.y = (float)hisy + (float)sy;
  pos.z = (float)hisz + (float)hisvz * t;
  if (targ->getStatus() & PlayerState::Falling) {
    pos.z += 0.5f * BZDBCache::gravity * t * t;
  }
  if (pos.z < 0.0f) {
    pos.z = 0.0f;
  }
}


// get coordinates to aim at when shooting a player; steps:
// 1. estimate how long it will take shot to hit target
// 2. calc position of target at that point of time
// 3. jump to 1., using projected position, loop until result is stable
void RobotPlayer::getProjectedPosition(const Player* targ, fvec3& projpos) const {
  double myx = getPosition()[0];
  double myy = getPosition()[1];
  double hisx = targ->getPosition()[0];
  double hisy = targ->getPosition()[1];
  double deltax = hisx - myx;
  double deltay = hisy - myy;
  double distance = hypotf(deltax, deltay) - BZDB.eval(BZDBNAMES.MUZZLEFRONT) - BZDBCache::tankRadius;
  if (distance <= 0) { distance = 0; }
  FlagType* fType = getFlagType();
  double shotspeed = BZDB.eval(BZDBNAMES.SHOTSPEED) *
                     (fType == Flags::Laser      ? BZDB.eval(BZDBNAMES.LASERADVEL) :
                      fType == Flags::RapidFire  ? BZDB.eval(BZDBNAMES.RFIREADVEL) :
                      fType == Flags::MachineGun ? BZDB.eval(BZDBNAMES.MGUNADVEL) : 1) +
                     hypotf(getVelocity()[0], getVelocity()[1]);

  double errdistance = 1.0;
  fvec3 tmpPos;
  for (int tries = 0 ; errdistance > 0.05 && tries < 4 ; tries++) {
    float t = (float)distance / (float)shotspeed;
    projectPosition(targ, t + 0.05f, tmpPos); // add 50ms for lag
    double distance2 = hypotf(tmpPos.x - myx, tmpPos.y - myy);
    errdistance = fabs(distance2 - distance) / (distance + ZERO_TOLERANCE);
    distance = distance2;
  }
  projpos = tmpPos;

  // projected pos in building -> use current pos
  if (World::getWorld() &&
      World::getWorld()->inBuilding(projpos, 0.0f, BZDBCache::tankHeight)) {
    projpos = targ->getPosition();
  }
}


void RobotPlayer::doUpdate(float dt) {
  LocalPlayer::doUpdate(dt);

  float tankRadius = BZDBCache::tankRadius;
  const float shotRange  = BZDB.eval(BZDBNAMES.SHOTRANGE);
  const float shotRadius = BZDBCache::shotRadius;

  World* world = World::getWorld();
  if (!world) {
    return;
  }

  // fire shot if any available
  timerForShot  -= dt;
  if (timerForShot < 0.0f) {
    timerForShot = 0.0f;
  }

  if (getFiringStatus() != Ready) {
    return;
  }

  bool  shoot   = false;
  const float azimuth = getAngle();
  // Allow shooting only if angle is near and timer has elapsed
  if ((int)path.size() != 0 && timerForShot <= 0.0f) {
    fvec3 p1;
    getProjectedPosition(target, p1);
    const fvec3& p2     = getPosition();
    float shootingAngle = atan2f(p1[1] - p2[1], p1[0] - p2[0]);
    if (shootingAngle < 0.0f) {
      shootingAngle += (float)(2.0 * M_PI);
    }
    float azimuthDiff   = shootingAngle - azimuth;
    if (azimuthDiff > M_PI) {
      azimuthDiff -= (float)(2.0 * M_PI);
    }
    else if (azimuthDiff < -M_PI) {
      azimuthDiff += (float)(2.0 * M_PI);
    }

    const float targetdistance = hypotf(p1[0] - p2[0], p1[1] - p2[1]) -
                                 BZDB.eval(BZDBNAMES.MUZZLEFRONT) - tankRadius;

    const float missby = fabs(azimuthDiff) *
                         (targetdistance - BZDBCache::tankLength);
    // only shoot if we miss by less than half a tanklength and no building inbetween
    if (missby < 0.5f * BZDBCache::tankLength &&
        p1[2] < shotRadius) {
      fvec3 pos = getPosition();
      pos.z +=  BZDB.eval(BZDBNAMES.MUZZLEHEIGHT);
      fvec3 dir(cosf(azimuth), sinf(azimuth), 0.0f);
      Ray tankRay(pos, dir);
      float maxdistance = targetdistance;
      if (!ShotStrategy::getFirstBuilding(tankRay, -0.5f, maxdistance)) {
        shoot = true;
        // try to not aim at teammates
        for (int i = 0; i <= world->getCurMaxPlayers(); i++) {
          Player* p = 0;
          if (i < world->getCurMaxPlayers()) {
            p = world->getPlayer(i);
          }
          else {
            p = LocalPlayer::getMyTank();
          }
          if (!p || p->getId() == getId() || validTeamTarget(p) ||
              !p->isAlive()) { continue; }
          const fvec3 relpos = getPosition() - p->getPosition();
          Ray ray(relpos, dir);
          float impact = Intersect::rayAtDistanceFromOrigin(ray, 5 * BZDBCache::tankRadius);
          if (impact > 0 && impact < shotRange) {
            shoot = false;
            timerForShot = 0.1f;
            break;
          }
        }
        if (shoot && fireShot()) {
          // separate shot by 0.2 - 0.8 sec (experimental value)
          timerForShot = float(bzfrand()) * 0.6f + 0.2f;
        }
      }
    }
  }
}

void RobotPlayer::doUpdateMotion(float dt) {
  if (isAlive()) {
    bool evading = false;
    // record previous position
    const float oldAzimuth = getAngle();
    const fvec3& oldPosition = getPosition();
    fvec3 position = oldPosition;
    float azimuth = oldAzimuth;
    float tankAngVel = BZDB.eval(BZDBNAMES.TANKANGVEL);
    float tankSpeed = BZDBCache::tankSpeed;

    World* world = World::getWorld();
    if (!world) {
      return;
    }

    // basically a clone of Roger's evasive code
    for (int t = 0; t <= world->getCurMaxPlayers(); t++) {
      Player* p = 0;
      if (t < world->getCurMaxPlayers()) {
        p = world->getPlayer(t);
      }
      else {
        p = LocalPlayer::getMyTank();
      }
      if (!p || p->getId() == getId()) {
        continue;
      }
      const int maxShots = p->getMaxShots();
      for (int s = 0; s < maxShots; s++) {
        ShotPath* shot = p->getShot(s);
        if (!shot || shot->isExpired()) {
          continue;
        }
        // ignore invisible bullets completely for now (even when visible)
        if ((shot->getShotType() == InvisibleShot) || (shot->getShotType() == CloakedShot)) {
          continue;
        }

        const fvec3& shotPos = shot->getPosition();
        if ((fabs(shotPos[2] - position[2]) > BZDBCache::tankHeight) && (shot->getShotType() != GMShot)) {
          continue;
        }
        const float dist = TargetingUtils::getTargetDistance(position, shotPos);
        if (dist < 150.0f) {
          const float* shotVel = shot->getVelocity();
          float shotAngle = atan2f(shotVel[1], shotVel[0]);
          float shotUnitVec[2] = {cosf(shotAngle), sinf(shotAngle)};

          float trueVec[2] = {(position[0] - shotPos[0]) / dist, (position[1] - shotPos[1]) / dist};
          float dotProd = trueVec[0] * shotUnitVec[0] + trueVec[1] * shotUnitVec[1];

          if (dotProd > 0.97f) {
            float rotation;
            float rotation1 = (float)((shotAngle + M_PI / 2.0) - azimuth);
            if (rotation1 < -1.0f * M_PI) { rotation1 += (float)(2.0 * M_PI); }
            if (rotation1 > 1.0f * M_PI) { rotation1 -= (float)(2.0 * M_PI); }

            float rotation2 = (float)((shotAngle - M_PI / 2.0) - azimuth);
            if (rotation2 < -1.0f * M_PI) { rotation2 += (float)(2.0 * M_PI); }
            if (rotation2 > 1.0f * M_PI) { rotation2 -= (float)(2.0 * M_PI); }

            if (fabs(rotation1) < fabs(rotation2)) {
              rotation = rotation1;
            }
            else {
              rotation = rotation2;
            }
            setDesiredSpeed(1.0f);
            setDesiredAngVel(rotation);
            evading = true;
          }
        }
      }
    }

    // when we are not evading, follow the path
    if (!evading && dt > 0.0 && pathIndex < (int)path.size()) {
      float distance;
      float v[2];
      const float* endPoint = path[pathIndex].get();
      // find how long it will take to get to next path segment
      v[0] = endPoint[0] - position[0];
      v[1] = endPoint[1] - position[1];
      distance = hypotf(v[0], v[1]);
      float tankRadius = BZDBCache::tankRadius;
      // smooth path a little by turning early at corners, might get us stuck, though
      if (distance <= 2.5f * tankRadius) {
        pathIndex++;
      }

      float segmentAzimuth = atan2f(v[1], v[0]);
      float azimuthDiff = segmentAzimuth - azimuth;
      if (azimuthDiff > M_PI) { azimuthDiff -= (float)(2.0 * M_PI); }
      else if (azimuthDiff < -M_PI) { azimuthDiff += (float)(2.0 * M_PI); }
      if (fabs(azimuthDiff) > 0.01f) {
        // drive backward when target is behind, try to stick to last direction
        if (drivingForward) {
          drivingForward = fabs(azimuthDiff) < M_PI / 2 * 0.9 ? true : false;
        }
        else {
          drivingForward = fabs(azimuthDiff) < M_PI / 2 * 0.3 ? true : false;
        }
        setDesiredSpeed(drivingForward ? 1.0f : -1.0f);
        // set desired turn speed
        if (azimuthDiff >= dt * tankAngVel) {
          setDesiredAngVel(1.0f);
        }
        else if (azimuthDiff <= -dt * tankAngVel) {
          setDesiredAngVel(-1.0f);
        }
        else {
          setDesiredAngVel(azimuthDiff / dt / tankAngVel);
        }
      }
      else {
        drivingForward = true;
        // tank doesn't turn while moving forward
        setDesiredAngVel(0.0f);
        // find how long it will take to get to next path segment
        if (distance <= dt * tankSpeed) {
          pathIndex++;
          // set desired speed
          setDesiredSpeed(distance / dt / tankSpeed);
        }
        else {
          setDesiredSpeed(1.0f);
        }
      }
    }
  }

  LocalPlayer::doUpdateMotion(dt);
}


void RobotPlayer::explodeTank() {
  LocalPlayer::explodeTank();
  target = NULL;
  path.clear();
}


void RobotPlayer::restart(const fvec3& pos, float _azimuth) {
  LocalPlayer::restart(pos, _azimuth);
  // no target
  path.clear();
  target = NULL;
  pathIndex = 0;

}


float RobotPlayer::getTargetPriority(const Player* _target) const {
  // don't target teammates or myself
  if (!this->validTeamTarget(_target)) {
    return 0.0f;
  }

  // go after closest player
  // FIXME -- this is a pretty stupid heuristic
  const float worldSize = BZDBCache::worldSize;
  const fvec3& p1 = getPosition();
  const fvec3& p2 = _target->getPosition();

  float basePriority = 1.0f;
  // give bonus to non-paused player
  if (!_target->isPaused()) {
    basePriority += 2.0f;
  }
  // give bonus to non-deadzone targets
  if (obstacleList) {
    fvec2 nearest;
    const BzfRegion* targetRegion = findRegion(p2.xy(), nearest);
    if (targetRegion && targetRegion->isInside(p2.xy())) {
      basePriority += 1.0f;
    }
  }
  return basePriority
         - 0.5f * hypotf(p2[0] - p1[0], p2[1] - p1[1]) / worldSize;
}


void RobotPlayer::setObstacleList(std::vector<BzfRegion*>* _obstacleList) {
  obstacleList = _obstacleList;
}


const Player* RobotPlayer::getTarget() const {
  return target;
}


void RobotPlayer::setTarget(const Player* _target) {
  static int mailbox = 0;

  path.clear();
  target = _target;
  if (!target) { return; }

  // work backwards (from target to me)
  fvec3 proj;
  getProjectedPosition(target, proj);
  const fvec2& p1 = proj.xy();
  const fvec2& p2 = getPosition().xy();
  fvec2 q1, q2;
  BzfRegion* headRegion = findRegion(p1, q1);
  BzfRegion* tailRegion = findRegion(p2, q2);
  if (!headRegion || !tailRegion) {
    // if can't reach target then forget it
    return;
  }

  mailbox++;
  headRegion->setPathStuff(0.0f, NULL, q1, mailbox);
  RegionPriorityQueue queue;
  queue.insert(headRegion, 0.0f);
  BzfRegion* next;
  while (!queue.isEmpty() && (next = queue.remove()) != tailRegion) {
    findPath(queue, next, tailRegion, q2, mailbox);
  }

  // get list of points to go through to reach the target
  next = tailRegion;
  do {
    path.push_back(next->getA());
    next = next->getTarget();
  }
  while (next && next != headRegion);
  if (next || tailRegion == headRegion) {
    path.push_back(q1);
  }
  else {
    path.clear();
  }
  pathIndex = 0;
}


BzfRegion* RobotPlayer::findRegion(const fvec2& p, fvec2& nearest) const {
  nearest[0] = p[0];
  nearest[1] = p[1];
  std::vector<BzfRegion*>::iterator itr;
  for (itr = obstacleList->begin(); itr != obstacleList->end(); ++itr)
    if ((*itr)->isInside(p)) {
      return (*itr);
    }

  // point is outside: find nearest region
  float distance = BzfRegion::maxDistance;
  BzfRegion* nearestRegion = NULL;
  for (itr = obstacleList->begin(); itr != obstacleList->end(); ++itr) {
    fvec2 currNearest;
    float currDistance = (*itr)->getDistance(p, currNearest);
    if (currDistance < distance) {
      nearestRegion = (*itr);
      distance = currDistance;
      nearest[0] = currNearest[0];
      nearest[1] = currNearest[1];
    }
  }
  return nearestRegion;
}


float RobotPlayer::getRegionExitPoint(const fvec2& p1, const fvec2& p2,
                                      const fvec2& a, const fvec2& targetPoint,
                                      fvec2& mid, float& priority) {
  fvec2 b;
  b[0] = targetPoint[0] - a[0];
  b[1] = targetPoint[1] - a[1];
  fvec2 d;
  d[0] = p2[0] - p1[0];
  d[1] = p2[1] - p1[1];

  float vect = d[0] * b[1] - d[1] * b[0];
  float t    = 0.0f;  // safe value
  if (fabs(vect) > ZERO_TOLERANCE) {
    // compute intersection along (p1,d) with (a,b)
    t = (a[0] * b[1] - a[1] * b[0] - p1[0] * b[1] + p1[1] * b[0]) / vect;
    if (t > 1.0f) {
      t = 1.0f;
    }
    else if (t < 0.0f) {
      t = 0.0f;
    }
  }

  mid[0] = p1[0] + t * d[0];
  mid[1] = p1[1] + t * d[1];

  const float distance = hypotf(a[0] - mid[0], a[1] - mid[1]);
  // priority is to minimize distance traveled and distance left to go
  priority = distance + hypotf(targetPoint[0] - mid[0], targetPoint[1] - mid[1]);
  // return distance traveled
  return distance;
}


void RobotPlayer::findPath(RegionPriorityQueue& queue, BzfRegion* region,
                           BzfRegion* targetRegion, const fvec2& targetPoint,
                           int mailbox) {
  const int numEdges = region->getNumSides();
  for (int i = 0; i < numEdges; i++) {
    BzfRegion* neighbor = region->getNeighbor(i);
    if (!neighbor) { continue; }

    const fvec2& p1 = region->getCorner(i).get();
    const fvec2& p2 = region->getCorner((i + 1) % numEdges).get();
    fvec2 mid;
    float priority;
    float total = getRegionExitPoint(p1, p2, region->getA(),
                                     targetPoint, mid, priority);
    priority += region->getDistance();
    if (neighbor == targetRegion) {
      total += hypotf(targetPoint[0] - mid[0], targetPoint[1] - mid[1]);
    }
    total += region->getDistance();
    if (neighbor->test(mailbox) || total < neighbor->getDistance()) {
      neighbor->setPathStuff(total, region, mid, mailbox);
      queue.insert(neighbor, priority);
    }
  }
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
