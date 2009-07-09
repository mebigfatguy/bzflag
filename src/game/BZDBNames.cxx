/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#if defined(_MSC_VER)
#pragma warning(4:4503)
#endif

// interface header
#include "BZDBNames.h"

// system headers
#include <string>

template <>
BZDBNames* Singleton<BZDBNames>::_instance = (BZDBNames*)NULL;

BZDBNames::BZDBNames()
: AGILITYADVEL             ("_agilityAdVel")
, AGILITYTIMEWINDOW        ("_agilityTimeWindow")
, AGILITYVELDELTA          ("_agilityVelDelta")
, AMBIENTLIGHT             ("_ambientLight")
, ANGLETOLERANCE           ("_angleTolerance")
, ANGULARAD                ("_angularAd")
, AUTOALLOWTIME            ("_autoAllowTime")
, AVENUESIZE               ("_avenueSize")
, BASESIZE                 ("_baseSize")
, BOXBASE                  ("_boxBase")
, BOXHEIGHT                ("_boxHeight")
, BURROWANGULARAD          ("_burrowAngularAd")
, BURROWDEPTH              ("_burrowDepth")
, BURROWSPEEDAD            ("_burrowSpeedAd")
, COLDETDEPTH              ("_coldetDepth")
, COLDETELEMENTS           ("_coldetElements")
, COUNTDOWNRESTIME         ("_countdownResumeTime")
, CULLDEPTH                ("_cullDepth")
, CULLELEMENTS             ("_cullElements")
, CULLOCCLUDERS            ("_cullOccluders")
, DISABLEBOTS              ("_disableBots")
, DRAWCELESTIAL            ("_drawCelestial")
, DRAWCLOUDS               ("_drawClouds")
, DRAWGROUND               ("_drawGround")
, DRAWGROUNDLIGHTS         ("_drawGroundLights")
, DRAWMOUNTAINS            ("_drawMountains")
, DRAWSKY                  ("_drawSky")
, ENDSHOTDETECTION         ("_endShotDetection")
, EXPLODETIME              ("_explodeTime")
, FLAGALTITUDE             ("_flagAltitude")
, FLAGEFFECTTIME           ("_flagEffectTime")
, FLAGHEIGHT               ("_flagHeight")
, FLAGPOLESIZE             ("_flagPoleSize")
, FLAGPOLEWIDTH            ("_flagPoleWidth")
, FLAGRADIUS               ("_flagRadius")
, FOGCOLOR                 ("_fogColor")
, FOGDENSITY               ("_fogDensity")
, FOGEND                   ("_fogEnd")
, FOGMODE                  ("_fogMode")
, FOGSTART                 ("_fogStart")
, FORBIDDEBUG              ("_forbidDebug")
, FRICTION                 ("_friction")
, GMACTIVATIONTIME         ("_gmActivationTime")
, GMADLIFE                 ("_gmAdLife")
, GMADSPEED                ("_gmAdSpeed")
, GMTURNANGLE              ("_gmTurnAngle")
, GRABOWNFLAG              ("_grabOwnFlag")
, GRAVITY                  ("_gravity")
, HANDICAPANGAD            ("_handicapAngAd")
, HANDICAPSCOREDIFF        ("_handicapScoreDiff")
, HANDICAPSHOTAD           ("_handicapShotAd")
, HANDICAPVELAD            ("_handicapVelAd")
, HEIGHTCHECKTOL           ("_heightCheckTolerance")
, IDENTIFYRANGE            ("_identifyRange")
, INERTIAANGULAR           ("_inertiaAngular")
, INERTIALINEAR            ("_inertiaLinear")
, JUMPVELOCITY             ("_jumpVelocity")
, LASERADLIFE              ("_laserAdLife")
, LASERADRATE              ("_laserAdRate")
, LASERADVEL               ("_laserAdVel")
, LATITUDE                 ("_latitude")
, LGGRAVITY                ("_lgGravity")
, LOCKONANGLE              ("_lockOnAngle")
, LONGITUDE                ("_longitude")
, LRADRATE                 ("_lRAdRate")
, MAXBUMPHEIGHT            ("_maxBumpHeight")
, MAXFLAGGRABS             ("_maxFlagGrabs")
, MAXLOD                   ("_maxLOD")
, MAXSELFDESTRUCTVEL       ("_maxSelfDestructVel")
, MGUNADLIFE               ("_mGunAdLife")
, MGUNADRATE               ("_mGunAdRate")
, MGUNADVEL                ("_mGunAdVel")
, MIRROR                   ("_mirror")
, MOMENTUMANGACC           ("_momentumAngAcc")
, MOMENTUMFRICTION         ("_momentumFriction")
, MOMENTUMLINACC           ("_momentumLinAcc")
, MUZZLEFRONT              ("_muzzleFront")
, MUZZLEHEIGHT             ("_muzzleHeight")
, NOCLIMB                  ("_noClimb")
, NOSHADOWS                ("_noShadows")
, NOSMALLPACKETS           ("_noSmallPackets")
, NOTRESPONDINGTIME        ("_notRespondingTime")
, OBESEFACTOR              ("_obeseFactor")
, PAUSEDROPTIME            ("_pauseDropTime")
, POSITIONTOLERANCE        ("_positionTolerance")
, PYRBASE                  ("_pyrBase")
, PYRHEIGHT                ("_pyrHeight")
, RADARLIMIT               ("_radarLimit")
, REJOINTIME               ("_rejoinTime")
, REJUMPTIME               ("_rejumpTime")
, RELOADTIME               ("_reloadTime")
, RFIREADLIFE              ("_rFireAdLife")
, RFIREADRATE              ("_rFireAdRate")
, RFIREADVEL               ("_rFireAdVel")
, SCOREBOARDCUSTOMFIELD    ("_scoreboardCustomRowField")
, SCOREBOARDCUSTOMROWLEN   ("_scoreboardCustomRowLen")
, SCOREBOARDCUSTOMROWNAME  ("_scoreboardCustomRowName")
, SHIELDFLIGHT             ("_shieldFlight")
, SHOCKADLIFE              ("_shockAdLife")
, SHOCKINRADIUS            ("_shockInRadius")
, SHOCKOUTRADIUS           ("_shockOutRadius")
, SHOTRADIUS               ("_shotRadius")
, SHOTRANGE                ("_shotRange")
, SHOTSKEEPVERTICALV       ("_shotsKeepVerticalVelocity")
, SHOTSPEED                ("_shotSpeed")
, SHOTTAILLENGTH           ("_shotTailLength")
, SPEEDCHECKSLOGONLY       ("_speedChecksLogOnly")
, SQUISHFACTOR             ("_squishFactor")
, SQUISHTIME               ("_squishTime")
, SRRADIUSMULT             ("_srRadiusMult")
, STARTINGRANK             ("_startingRank")
, SYNCLOCATION             ("_syncLocation")
, SYNCTIME                 ("_syncTime")
, TANKANGVEL               ("_tankAngVel")
, TANKEXPLOSIONSIZE        ("_tankExplosionSize")
, TANKHEIGHT               ("_tankHeight")
, TANKLENGTH               ("_tankLength")
, TANKRADIUS               ("_tankRadius")
, TANKSHOTPROXIMITY        ("_tankShotProximity")
, TANKSPEED                ("_tankSpeed")
, TANKWIDTH                ("_tankWidth")
, TARGETINGANGLE           ("_targetingAngle")
, TCPTIMEOUT               ("_tcpTimeout")
, TELEBREADTH              ("_teleportBreadth")
, TELEHEIGHT               ("_teleportHeight")
, TELEPORTTIME             ("_teleportTime")
, TELEWIDTH                ("_teleportWidth")
, THIEFADLIFE              ("_thiefAdLife")
, THIEFADRATE              ("_thiefAdRate")
, THIEFADSHOTVEL           ("_thiefAdShotVel")
, THIEFDROPTIME            ("_thiefDropTime")
, THIEFTINYFACTOR          ("_thiefTinyFactor")
, THIEFVELAD               ("_thiefVelAd")
, TINYFACTOR               ("_tinyFactor")
, TRACKFADE                ("_trackFade")
, UPDATETHROTTLERATE       ("_updateThrottleRate")
, VELOCITYAD               ("_velocityAd")
, WALLHEIGHT               ("_wallHeight")
, WEAPONS                  ("_weapons")
, WIDEANGLEANG             ("_wideAngleAng")
, WINGSGRAVITY             ("_wingsGravity")
, WINGSJUMPCOUNT           ("_wingsJumpCount")
, WINGSJUMPVELOCITY        ("_wingsJumpVelocity")
, WINGSSLIDETIME           ("_wingsSlideTime")
, WORLDSIZE                ("_worldSize")
{
  // do nothing
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
