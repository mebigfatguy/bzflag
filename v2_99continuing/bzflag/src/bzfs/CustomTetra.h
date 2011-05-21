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

#ifndef __CUSTOMTETRA_H__
#define __CUSTOMTETRA_H__

/* interface header */
#include "WorldFileObstacle.h"

/* system header */
#include <string>

/* common interface header */
#include "BzMaterial.h"
#include "vectors.h"

/* local interface header */
#include "WorldInfo.h"

class CustomTetra : public WorldFileObstacle {
  public:
    CustomTetra();
    virtual bool read(const char* cmd, std::istream& input);
    virtual void writeToGroupDef(GroupDefinition*) const;

  private:
    int vertexCount;

    fvec3 vertices[4];
    fvec3 normals[4];
    fvec2 texcoords[4];
    bool useNormals[4];
    bool useTexcoords[4];
    BzMaterial materials[4];
};

#endif  /* __CUSTOMTETRA_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
