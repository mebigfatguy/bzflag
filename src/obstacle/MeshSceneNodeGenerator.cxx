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

#include "vectors.h"
#include "MeshSceneNodeGenerator.h"
#include "MeshObstacle.h"
#include "MeshFace.h"
#include "bzfgl.h"
#include "MeshPolySceneNode.h"
#include "DynamicColor.h"
#include "TextureManager.h"
#include "OpenGLMaterial.h"


//
// MeshSceneNodeGenerator
//

MeshSceneNodeGenerator::MeshSceneNodeGenerator(const MeshObstacle* _mesh)
{
  mesh = _mesh;
  faceNumber = 0;
  return;
}

MeshSceneNodeGenerator::~MeshSceneNodeGenerator()
{
  // do nothing
  return;
}

WallSceneNode* MeshSceneNodeGenerator::getNextNode(bool /*lod*/)
{
  const MeshFace* face;
  const BzMaterial* mat;

  // remove any faces on the ground that will not be displayed
  // also, return NULL if we are at the end of the face list
  while (true) {
    if (faceNumber >= mesh->getFaceCount()) {
      return NULL;
    }
    face = mesh->getFace(faceNumber);
    mat = face->getMaterial();
    const DynamicColor* dyncol = DYNCOLORMGR.getColor(mat->getDynamicColor());
    if ((mat->getDiffuse()[3] == 0.0f) && (dyncol == NULL) &&
        !((mat->getTextureCount() > 0) && !mat->getUseColorOnTexture(0))) {
      // face is invisible
      faceNumber++;
      continue;
    }
    const float* plane = face->getPlane();
    if (plane[2] < -0.9f) {
      // plane is facing downwards
      float mins[3], maxs[3];
      face->getExtents(mins, maxs);
      if (maxs[2] < 0.001) {
        // plane is on or below the ground, ditch it
        faceNumber++;
        continue;
      } else {
        break;
      }
    } else {
      break;
    }
  }

  MeshPolySceneNode* node = getSceneNode(face);

  setupNodeMaterial(node, mat);

  faceNumber++;

  return node;
}


MeshPolySceneNode* MeshSceneNodeGenerator::getSceneNode(const MeshFace* face)
{
  int i;
  
  // vertices
  const int vertexCount = face->getVertexCount();
  GLfloat3Array vertices(vertexCount);
  for (i = 0; i < vertexCount; i++) {
    memcpy (vertices[i], face->getVertex(i), sizeof(float[3]));
  }

  // normals
  int normalCount = 0;
  if (face->useNormals()) {
    normalCount = vertexCount;
  }
  GLfloat3Array normals(normalCount);
  for (i = 0; i < normalCount; i++) {
    memcpy (normals[i], face->getNormal(i), sizeof(float[3]));
  }

  // texcoords
  GLfloat2Array texcoords(vertexCount);
  if (face->useTexcoords()) {
    for (i = 0; i < vertexCount; i++) {
      memcpy (texcoords[i], face->getTexcoord(i), sizeof(float[2]));
    }
  } else {
    makeTexcoords (face->getPlane(), vertices, texcoords);
  }

  MeshPolySceneNode* node =
    new MeshPolySceneNode(face->getPlane(), vertices, normals, texcoords);
    
  return node;
}


void MeshSceneNodeGenerator::setupNodeMaterial(MeshPolySceneNode* node,
                                               const BzMaterial* mat)
{
  TextureManager &tm = TextureManager::instance();
  OpenGLMaterial glMaterial(mat->getSpecular(), mat->getEmission(), mat->getShininess());

  int userTexture = (mat->getTextureCount() > 0);
  int faceTexture = -1;
  bool gotSpecifiedTexture = false;
  if (userTexture) {
    if (mat->getTexture(0).size() > 0) {
      faceTexture = tm.getTextureID(mat->getTexture(0).c_str());
    }
    if (faceTexture >= 0) {
      gotSpecifiedTexture = true;
    } else {
      faceTexture = tm.getTextureID("mesh", false /* no failure reports */);
    }
  }

  // NOTE: the diffuse color is used, and not the ambient color
  //       could use the ambient color for non-lighted,and diffuse
  //       for lighted
  const DynamicColor* dyncol = DYNCOLORMGR.getColor(mat->getDynamicColor());
  const GLfloat* dc = NULL;
  if (dyncol != NULL) {
    dc = dyncol->getColor();
  }
  node->setDynamicColor(dc);
  node->setColor(mat->getDiffuse()); // redundant, see below
  node->setModulateColor(mat->getDiffuse());
  node->setLightedColor(mat->getDiffuse());
  node->setLightedModulateColor(mat->getDiffuse());
  node->setMaterial(glMaterial);
  node->setTexture(faceTexture);
  if ((userTexture && mat->getUseColorOnTexture(0)) ||
      !gotSpecifiedTexture) {
    // modulate with the color if asked to, or
    // if the specified texture was not available
    node->setUseColorTexture(false);
  } else {
    node->setUseColorTexture(true);
  }
  node->setTextureMatrix(mat->getTextureMatrix(0));

  // deal with the blending setting for textures
  bool alpha = false;
  if ((faceTexture >= 0) && (userTexture && mat->getUseTextureAlpha(0))) {
    const ImageInfo& imageInfo = tm.getInfo(faceTexture);
    alpha = imageInfo.alpha;
  }
  node->setBlending(alpha);
  node->setSphereMap(mat->getUseSphereMap(0));

  // the current color can also affect the blending.
  // if blending is disabled then the alpha value from
  // one of these colors is used to set the stipple value.
  // we'll just set it to the middle value.
  if (dyncol) {
    const float color[4] = { 1.0f, 1.0f, 1.0f, 0.5f }; // alpha value != 1.0f
    if (dyncol->canHaveAlpha()) {
      node->setColor(color); // trigger transparency check
      node->setModulateColor(color);
      node->setLightedColor(color);
      node->setLightedModulateColor(color);
    }
  }
}


bool MeshSceneNodeGenerator::makeTexcoords(const float* plane,
                                           const GLfloat3Array& vertices,
                                           GLfloat2Array& texcoords)
{
  float x[3], y[3];

  vec3sub (x, vertices[1], vertices[0]);
  vec3cross (y, plane, x);

  float len = vec3dot(x, x);
  if (len > 0.0f) {
    len = 1 / len;
    x[0] = x[0] * len;
    x[1] = x[1] * len;
    x[2] = x[2] * len;
  } else {
    return false;
  }

  len = vec3dot(y, y);
  if (len > 0.0f) {
    len = 1 / len;
    y[0] = y[0] * len;
    y[1] = y[1] * len;
    y[2] = y[2] * len;
  } else {
    return false;
  }

  const float uvScale = 4.0f;

  texcoords[0][0] = 0.0f;
  texcoords[0][1] = 0.0f;
  const int count = vertices.getSize();
  for (int i = 1; i < count; i++) {
    float delta[3];
    vec3sub (delta, vertices[i], vertices[0]);
    texcoords[i][0] = vec3dot(delta, x) / uvScale;
    texcoords[i][1] = vec3dot(delta, y) / uvScale;
  }

  return true;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
