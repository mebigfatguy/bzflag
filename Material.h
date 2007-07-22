/* bzflag
 * Copyright (c) 1993 - 2006 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include <string>
#include <vector>
#include <fstream> 

class Material {
  std::string name;
  std::string file;
public:
  Material(const std::string& _name, const std::string& _file) : name(_name), file(_file) {};
  void output(std::ofstream& out);
};

typedef std::vector<Material*> MaterialVector;
typedef MaterialVector::iterator MaterialVectIter;

#endif /* __MATERIAL_H__ */

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
