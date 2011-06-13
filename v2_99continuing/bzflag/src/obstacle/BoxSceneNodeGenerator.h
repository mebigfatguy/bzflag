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

#ifndef __BOXSCENENODEGENERATOR_H__
#define __BOXSCENENODEGENERATOR_H__

#include "obstacle/BoxBuilding.h"
#include "obstacle/ObstacleSceneNodeGenerator.h"

class BoxSceneNodeGenerator : public ObstacleSceneNodeGenerator {
    friend class SceneDatabaseBuilder;
  public:
    ~BoxSceneNodeGenerator();

    WallSceneNode*  getNextNode(float, float, bool);

  protected:
    BoxSceneNodeGenerator(const BoxBuilding*);

  private:
    const BoxBuilding*  box;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
