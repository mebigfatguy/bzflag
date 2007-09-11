/* bzflag
 * Copyright (c) 1993 - 2007 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef __CLIENTINTANGIBILITYMANAGER_H__
#define __CLIENTINTANGIBILITYMANAGER_H__

#include "common.h"
#include "Singleton.h"
#include <map>

#define _INVALID_TANGIBILITY 255

class ClientIntangibilityManager :   public Singleton<ClientIntangibilityManager>
{
public:
  void setWorldObjectTangibility ( unsigned int objectGUID, unsigned char tangible );
    
  void resetTangibility ( void );

  unsigned char getWorldObjectTangibility ( unsigned int objectGUID );

protected:
  friend class Singleton<ClientIntangibilityManager>;

private:
  ClientIntangibilityManager(){};
  ~ClientIntangibilityManager(){};

  std::map<unsigned int, unsigned char> tangibilityMap;
};

#endif  /*__CLIENTINTANGIBILITYMANAGER_H__ */

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
