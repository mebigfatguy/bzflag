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

#ifndef __DROP_GEOMETRY_H__
#define __DROP_GEOMETRY_H__


class WorldInfo;


namespace DropGeometry {

  bool dropFlag (float pos[3], float minZ, float maxZ,
		 const WorldInfo* world);
  bool dropPlayer (float pos[3], float minZ, float maxZ,
		   const WorldInfo* world);
  bool dropTeamFlag (float pos[3], float minZ, float maxZ,
		     const WorldInfo* world, int team);
}


#endif  /* __DROP_GEOMETRY_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
