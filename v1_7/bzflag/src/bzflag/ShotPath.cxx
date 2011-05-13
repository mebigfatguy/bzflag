/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "common.h"
#include "ShotPath.h"
#include "ShotStrategy.h"
#include "LocalPlayer.h"
#include "Protocol.h"
#include "PlayerLink.h"
#include "playing.h"

//
// FiringInfo (with BaseLocalPlayer)
//

FiringInfo::FiringInfo(const BaseLocalPlayer& tank, int id)
{
  shot.player = tank.getId();
  shot.id = uint16_t(id);
  tank.getMuzzle(shot.pos);
  const float* dir = tank.getForward();
  const float* tankVel = tank.getVelocity();
  shot.vel[0] = tankVel[0] + ShotSpeed * dir[0];
  shot.vel[1] = tankVel[1] + ShotSpeed * dir[1];
  shot.vel[2] = tankVel[2] + ShotSpeed * dir[2];
  shot.dt = 0.0f;
  flag = tank.getFlag();
  lifetime = ReloadTime;
}

//
// ShotPath
//

ShotPath::ShotPath(const FiringInfo& info) :
				firingInfo(info),
				reloadTime(ReloadTime),
				startTime(TimeKeeper::getTick()),
				currentTime(TimeKeeper::getTick()),
				expiring(false),
				expired(false)
{
  // eek!  a giant switch statement, how un-object-oriented!
  // each flag should be a flyweight object derived from a
  // base Flag class with a virtual makeShotStrategy() member.
  // just remember -- it's only a game.
  switch (firingInfo.flag) {
    case NoFlag:
    case RedFlag:
    case GreenFlag:
    case BlueFlag:
    case PurpleFlag:
    case VelocityFlag:
    case QuickTurnFlag:
    case OscOverthrusterFlag:
    case InvisibleBulletFlag:
    case StealthFlag:
    case TinyFlag:
    case NarrowFlag:
    case ShieldFlag:
    case SteamrollerFlag:
    case IdentifyFlag:
    case CloakingFlag:
    case PhantomZoneFlag:
    case JumpingFlag:
    case GenocideFlag:
    case ColorblindnessFlag:
    case ObesityFlag:
    case LeftTurnOnlyFlag:
    case RightTurnOnlyFlag:
    case MomentumFlag:
    case BlindnessFlag:
    case JammingFlag:
    case WideAngleFlag:
      strategy = new NormalShotStrategy(this);
      break;
    case RapidFireFlag:
      strategy = new RapidFireStrategy(this);
      break;
    case MachineGunFlag:
      strategy = new MachineGunStrategy(this);
      break;
    case GuidedMissileFlag:
      strategy = new GuidedMissileStrategy(this);
      break;
    case LaserFlag:
      strategy = new LaserStrategy(this);
      break;
    case RicochetFlag:
      strategy = new RicochetStrategy(this);
      break;
    case SuperBulletFlag:
      strategy = new SuperBulletStrategy(this);
      break;
    case ShockWaveFlag:
      strategy = new ShockWaveStrategy(this);
      break;
    default:
      // shouldn't happen
      assert(0);
      break;
  }
}

ShotPath::~ShotPath()
{
  delete strategy;
}

float			ShotPath::checkHit(const BaseLocalPlayer* player,
							float position[3]) const
{
  return strategy->checkHit(player, position);
}

bool			ShotPath::isStoppedByHit() const
{
  return strategy->isStoppedByHit();
}

void			ShotPath::addShot(SceneDatabase* scene,
						bool colorblind)
{
  strategy->addShot(scene, colorblind);
}

void			ShotPath::radarRender() const
{
  if (!isExpired()) strategy->radarRender();
}

void			ShotPath::updateShot(float dt)
{
  // get new time step and set current time
  currentTime += dt;

  // update shot
  if (!expired)
    if (expiring) setExpired();
    else getStrategy()->update(dt);
}

void			ShotPath::setReloadTime(float _reloadTime)
{
  reloadTime = _reloadTime;
}

void			ShotPath::setPosition(const float* p)
{
  firingInfo.shot.pos[0] = p[0];
  firingInfo.shot.pos[1] = p[1];
  firingInfo.shot.pos[2] = p[2];
}

void			ShotPath::setVelocity(const float* v)
{
  firingInfo.shot.vel[0] = v[0];
  firingInfo.shot.vel[1] = v[1];
  firingInfo.shot.vel[2] = v[2];
}

void			ShotPath::setExpiring()
{
  expiring = true;
}

void			ShotPath::setExpired()
{
  expiring = true;
  expired = true;
  getStrategy()->expire();
}

void			ShotPath::boostReloadTime(float dt)
{
  reloadTime += dt;
}

//
// LocalShotPath
//

LocalShotPath::LocalShotPath(const FiringInfo& info) :
				ShotPath(info)
{
  // do nothing
}

LocalShotPath::~LocalShotPath()
{
  // do nothing
}

void			LocalShotPath::update(float dt)
{
  getFiringInfo().shot.dt += dt;
  updateShot(dt);

  // send updates if necessary
  if (PlayerLink::getMulticast())
    getStrategy()->sendUpdate(getFiringInfo());
}

//
// RemoteShotPath
//

RemoteShotPath::RemoteShotPath(const FiringInfo& info) :
				ShotPath(info)
{
  // do nothing
}

RemoteShotPath::~RemoteShotPath()
{
  // do nothing
}

void			RemoteShotPath::update(float dt)
{
  // update shot
  updateShot(dt);
}

void			RemoteShotPath::update(const ShotUpdate& shot,
				uint16_t code, void* msg)
{
  // update shot info
  getFiringInfo().shot = shot;

  // let the strategy see the message
  getStrategy()->readUpdate(code, msg);
}
// ex: shiftwidth=2 tabstop=8