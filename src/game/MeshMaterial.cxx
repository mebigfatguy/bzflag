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

#include <string.h>

#include "MeshMaterial.h"
#include "Pack.h"


void MeshMaterial::reset()
{
  texture = "mesh";
  textureMatrix = -1;
  dynamicColor = -1;
  const float defAmbient[4] = { 0.2f, 0.2f, 0.2f, 1.0f };
  const float defDiffuse[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
  const float defSpecular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  const float defEmission[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  memcpy (ambient, defAmbient, sizeof(ambient));
  memcpy (diffuse, defDiffuse, sizeof(diffuse));
  memcpy (specular, defSpecular, sizeof(specular));
  memcpy (emission, defEmission, sizeof(emission));
  shininess = 0.0f;
  useColor = true;
  return;
}


MeshMaterial::MeshMaterial()
{
  reset();
  return;
}


MeshMaterial::MeshMaterial(const MeshMaterial& m)
{
  texture = m.texture;
  textureMatrix = m.textureMatrix;
  dynamicColor = m.dynamicColor;
  memcpy (ambient, m.ambient, sizeof(ambient));
  memcpy (diffuse, m.diffuse, sizeof(diffuse));
  memcpy (specular, m.specular, sizeof(specular));
  memcpy (emission, m.emission, sizeof(emission));
  shininess = m.shininess;
  useColor = m.useColor;
  return;
}


MeshMaterial& MeshMaterial::operator=(const MeshMaterial& m)
{
  texture = m.texture;
  textureMatrix = m.textureMatrix;
  dynamicColor = m.dynamicColor;
  memcpy (ambient, m.ambient, sizeof(ambient));
  memcpy (diffuse, m.diffuse, sizeof(diffuse));
  memcpy (specular, m.specular, sizeof(specular));
  memcpy (emission, m.emission, sizeof(emission));
  shininess = m.shininess;
  useColor = m.useColor;
  return *this;
}


bool MeshMaterial::operator==(const MeshMaterial& m)
{
  if ((texture != m.texture) ||
      (textureMatrix != m.textureMatrix) ||
      (dynamicColor != m.dynamicColor) ||
      (shininess != m.shininess) ||
      (useColor != m.useColor)) {
    return false;
  }
  for (int i = 0; i < 4; i++) {
    if ((ambient[i] != m.ambient[i]) ||
        (diffuse[i] != m.diffuse[i]) ||
        (specular[i] != m.specular[i]) ||
        (emission[i] != m.emission[i])) {
      return false;
    }
  }
  return true;
}


static void* pack4Float(void *buf, const float values[4])
{
  int i;
  for (i = 0; i < 4; i++) {
    buf = nboPackFloat(buf, values[i]);
  }
  return buf;
}


static void* unpack4Float(void *buf, float values[4])
{
  int i;
  for (i = 0; i < 4; i++) {
    buf = nboUnpackFloat(buf, values[i]);
  }
  return buf;
}


void* MeshMaterial::pack(void* buf)
{
  unsigned char len = texture.size();
  buf = nboPackUByte(buf, len);
  buf = nboPackString(buf, texture.c_str(), len);
  buf = nboPackInt(buf, textureMatrix);
  buf = nboPackInt(buf, dynamicColor);
  buf = pack4Float(buf, ambient);
  buf = pack4Float(buf, diffuse);
  buf = pack4Float(buf, specular);
  buf = pack4Float(buf, emission);
  buf = nboPackFloat(buf, shininess);
  return buf;
}


void* MeshMaterial::unpack(void* buf)
{
  char textureStr[256];
  unsigned char len;
  buf = nboUnpackUByte(buf, len);
  buf = nboUnpackString(buf, textureStr, len);
  textureStr[len] = '\0';
  texture = textureStr;
  buf = nboUnpackInt(buf, textureMatrix);
  buf = nboUnpackInt(buf, dynamicColor);
  buf = unpack4Float(buf, ambient);
  buf = unpack4Float(buf, diffuse);
  buf = unpack4Float(buf, specular);
  buf = unpack4Float(buf, emission);
  buf = nboUnpackFloat(buf, shininess);
  return buf;
}


int MeshMaterial::packSize()
{
  const int basicSize = sizeof(unsigned char) + sizeof(int) + sizeof(int) +
                        (4 * sizeof(float[4])) + sizeof(float);
  unsigned char len = texture.size();
  return basicSize + len;
}


void MeshMaterial::print(std::ostream& out, int /*level*/)
{
  out << "    texture " << texture << std::endl;
  out << "    texmat " << textureMatrix << std::endl;
  out << "    dyncol " << dynamicColor << std::endl;
  out << "    ambient " << ambient[0] << " " << ambient[1] << " "
                        << ambient[2] << " " << ambient[3] << std::endl;
  out << "    diffuse " << diffuse[0] << " " << diffuse[1] << " "
                        << diffuse[2] << " " << diffuse[3] << std::endl;
  out << "    specular " << specular[0] << " " << specular[1] << " "
                         << specular[2] << " " << specular[3] << std::endl;
  out << "    emission " << emission[0] << " " << emission[1] << " "
                         << emission[2] << " " << emission[3] << std::endl;
  out << "    shininess " << shininess << std::endl;
  return;
}


// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

