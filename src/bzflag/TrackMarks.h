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

#ifndef BZF_TRACKS_H
#define BZF_TRACKS_H


namespace TrackMarks {

  void init();
  void kill();
  void clear();
  void render();
  void update(float dt);

  bool addMark(const float pos[3], float scale, float angle, int phydrv);

  void setUserFade(float);
  float getUserFade();
  
  enum AirCullStyle {
    NoAirCull     = 0,
    InitAirCull   = (1 << 0), // cull for initial air mark conditions
    PhyDrvAirCull = (1 << 1), // cull for physics driver effects
    FullAirCull   = (InitAirCull | PhyDrvAirCull)
  };

  void setAirCulling(AirCullStyle style);
  AirCullStyle getAirCulling();
  
  const float updateTime = (1.0f / 20.0f);
}


#endif // BZF_TRACKS_H


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

