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

#ifndef __MESHSCENENODEGENERATOR_H__
#define __MESHSCENENODEGENERATOR_H__

#include "MeshObstacle.h"
#include "SceneNode.h"
#include "MeshMaterial.h"

class WallSceneNode;
class MeshPolySceneNode;

class MeshSceneNodeGenerator {

  friend class SceneDatabaseBuilder;
  
  public:
    ~MeshSceneNodeGenerator();
    WallSceneNode* getNextNode(float, float, bool);
    static void setupNodeMaterial(MeshPolySceneNode* node,
                                  const MeshMaterial* mat);
  protected:
    MeshSceneNodeGenerator(const MeshObstacle*);

  private:
    bool makeTexcoords(const float* plane, 
                       const GLfloat3Array& vertices,
                       GLfloat2Array& texcoords);
    int faceNumber;
    const MeshObstacle* mesh;
};

#endif

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
