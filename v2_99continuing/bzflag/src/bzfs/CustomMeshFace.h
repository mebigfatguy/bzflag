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

#ifndef __CUSTOM_MESH_FACE_H__
#define __CUSTOM_MESH_FACE_H__

#include "common.h"

/* system header */
#include <string>
#include <vector>

/* common interface header */
#include "game/BzMaterial.h"
#include "obstacle/MeshFace.h"
#include "obstacle/MeshObstacle.h"


class CustomMeshFace {
  public:
    CustomMeshFace(const BzMaterial& material, int phydrv, bool noclusters,
                   bool smoothBounce, unsigned char driveThrough, unsigned char shootThrough);
    bool read(const char* cmd, std::istream& input);
    void write(MeshObstacle* mesh) const;

  private:
    BzMaterial material;

    std::vector<int> vertices;
    std::vector<int> normals;
    std::vector<int> texcoords;

    int  phydrv;
    bool noclusters;
    bool smoothBounce;
    unsigned char driveThrough;
    unsigned char shootThrough;
    bool          ricochet;

    MeshFace::SpecialData specialData;
};


#endif  /* __CUSTOM_MESH_FACE_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
