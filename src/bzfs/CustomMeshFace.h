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

#ifndef __CUSTOM_MESH_FACE_H__
#define __CUSTOM_MESH_FACE_H__

/*  interface header */
#include "CustomMeshFace.h"

/* common interface header */
#include "MeshMaterial.h"
#include "MeshObstacle.h"
#include "MeshFace.h"

/* system header */
#include <string>
#include <vector>


class CustomMeshFace {
  public:
    CustomMeshFace(const MeshMaterial& material);
    bool read(const char *cmd, std::istream& input);
    void write(MeshObstacle* mesh) const;

  private:
    MeshMaterial material;
    std::vector<int> vertices;
    std::vector<int> normals;
    std::vector<int> texcoords;
};


#endif  /* __CUSTOM_MESH_FACE_H__ */

// Local variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
