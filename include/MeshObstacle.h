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

/* TetraBuilding:
 *	Encapsulates a tetrahederon in the game environment.
 */

#ifndef	BZF_MESH_OBSTACLE_H
#define	BZF_MESH_OBSTACLE_H

#include "common.h"
#include <string>
#include <vector>
#include <iostream>
#include "vectors.h"
#include "Ray.h"
#include "Obstacle.h"
#include "MeshFace.h"


class MeshObstacle : public Obstacle {
  public:
    MeshObstacle();
    MeshObstacle(const std::vector<char>& checkTypes,
                 const std::vector<cfvec3>& checkPoints,
                 const std::vector<cfvec3>& vertices,
                 const std::vector<cfvec3>& normals,
                 const std::vector<cfvec2>& texcoords,
                 int faceCount,
                 bool bounce, bool drive, bool shoot);

    bool addFace (const std::vector<int>& vertices,
                  const std::vector<int>& normals,
                  const std::vector<int>& texcoords,
                  const BzMaterial* bzMaterial, int physics,
                  bool bounce, bool drive, bool shoot);

    ~MeshObstacle();

    void finalize();

    enum CheckType {
      CheckInside = 0,
      CheckOutside = 1
    };

    const char* getType() const;
    static const char* getClassName(); // const
    bool isValid() const;
    void getExtents(float* mins, float* maxs) const;

    float intersect(const Ray&) const;
    void getNormal(const float* p, float* n) const;
    void get3DNormal(const float* p, float* n) const;

    bool inCylinder(const float* p, float radius, float height) const;
    bool inBox(const float* p, float angle,
               float halfWidth, float halfBreadth, float height) const;
    bool inMovingBox(const float* oldP, float oldAngle,
                     const float *newP, float newAngle,
                     float halfWidth, float halfBreadth, float height) const;
    bool isCrossing(const float* p, float angle,
                    float halfWidth, float halfBreadth, float height,
                    float* plane) const;

    bool getHitNormal(const float* pos1, float azimuth1,
                      const float* pos2, float azimuth2,
                      float halfWidth, float halfBreadth,
                      float height, float* normal) const;

    const char *getCheckTypes() const;
    const fvec3 *getCheckPoints() const;
    const fvec3 *getVertices() const;
    const fvec3 *getNormals() const;
    const fvec2 *getTexcoords() const;
    const int getVertexCount() const;
    const int getNormalCount() const;
    const int getTexcoordCount() const;
    int getFaceCount() const;
    const MeshFace* getFace(int face) const;
    bool useSmoothBounce() const;

    bool useFragments() const { return fragments; }
    void setFragments(bool value) { fragments = value; }
    
    void setIsLocal(bool);
    bool getIsLocal() const;

    void *pack(void*);
    void *unpack(void*);
    int packSize();

    void print(std::ostream& out, int level);

  private:

    static const char* typeName;
    bool isLocal; // generated on the client side
    
    int checkCount;
    char* checkTypes;
    fvec3* checkPoints;
    int vertexCount;
    fvec3* vertices;
    int normalCount;
    fvec3* normals;
    int texcoordCount;
    fvec2* texcoords;
    int faceCount, faceSize;
    MeshFace** faces;
    bool smoothBounce;
    bool fragments;

    fvec3 mins, maxs;
};

inline const char *MeshObstacle::getCheckTypes() const
{
  return checkTypes;
}

inline const fvec3 *MeshObstacle::getCheckPoints() const
{
  return checkPoints;
}

inline const fvec3 *MeshObstacle::getVertices() const
{
  return vertices;
}

inline const fvec3 *MeshObstacle::getNormals() const
{
  return normals;
}

inline const fvec2 *MeshObstacle::getTexcoords() const
{
  return texcoords;
}

inline const int MeshObstacle::getVertexCount() const
{
  return vertexCount;
}

inline const int MeshObstacle::getNormalCount() const
{
  return normalCount;
}

inline const int MeshObstacle::getTexcoordCount() const
{
  return texcoordCount;
}

inline int MeshObstacle::getFaceCount() const
{
  return faceCount;
}

inline const MeshFace* MeshObstacle::getFace(int face) const
{
  return faces[face];
}

inline bool MeshObstacle::useSmoothBounce() const
{
  return smoothBounce;
}

inline void MeshObstacle::setIsLocal(bool value)
{
  isLocal = value;
  return;
}

inline bool MeshObstacle::getIsLocal() const
{
  return isLocal;
}


#endif // BZF_MESH_OBSTACLE_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

