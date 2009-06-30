/* bzflag
 * Copyright (c) 1993 - 2009 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef _PHYSICS_DRIVER_H_
#define _PHYSICS_DRIVER_H_

#include "common.h"

// system headers
#include <string>
#include <vector>
#include <set>
#include <map>
#include <iostream>

// common headers
#include "vectors.h"


class PhysicsDriver {

  friend class PhysicsDriverManager;

  public:
    PhysicsDriver();
    ~PhysicsDriver();

    bool setName(const std::string& name);

    void setLinear(const fvec3& vel);
    void setLinearVar(const std::string& varName);

    void setAngular(float angleVel, const fvec2& pos);
    void setAngularVar(const std::string& varName);

    void setRadial(float radialVel, const fvec2& pos);
    void setRadialVar(const std::string& varName);

    void setSlideTime(float slideTime);
    void setSlideVar(const std::string& varName);

    void setDeathMessage(const std::string& msg);
    void setDeathVar(const std::string& varName);

    void finalize();
    void update(float time);

    inline const std::string& getName() const { return name; }
    inline int                getID()   const { return id; }
    inline const fvec3& getLinearVel()  const { return linearVel; }
    inline float        getAngularVel() const { return angularVel; }
    inline const fvec2& getAngularPos() const { return angularPos; }
    inline float        getRadialVel()  const { return radialVel; }
    inline const fvec2& getRadialPos()  const { return radialPos; }
    inline bool         getIsSlide()    const { return slideTime > 0.0f; }
    inline float        getSlideTime()  const { return slideTime; }
    inline bool         getIsDeath()    const { return !deathMsg.empty(); }
    inline bool         getPossibleDeath() const {
      return !deathMsg.empty() || !deathVar.empty();
    }
    inline const std::string& getDeathMsg() const { return deathMsg; }

    bool operator<(const PhysicsDriver& pd) const;    

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    static void staticLinearCallback(const std::string& name, void* data);
    static void staticAngularCallback(const std::string& name, void* data);
    static void staticRadialCallback(const std::string& name, void* data);
    static void staticSlideCallback(const std::string& name, void* data);
    static void staticDeathCallback(const std::string& name, void* data);
    void linearCallback(const std::string& name);
    void angularCallback(const std::string& name);
    void radialCallback(const std::string& name);
    void slideCallback(const std::string& name);
    void deathCallback(const std::string& name);

  private:
    std::string name;
    int id;

    fvec3 linearVel;
    std::string linearVar;

    float angularVel;
    fvec2 angularPos;
    std::string angularVar;

    float radialVel;
    fvec2 radialPos;
    std::string radialVar;

    float slideTime;
    std::string slideVar;
    
    std::string deathMsg;
    std::string deathVar;
};


class PhysicsDriverManager {
  public:
    PhysicsDriverManager();
    ~PhysicsDriverManager();
    void update();
    void clear();
    int addDriver(PhysicsDriver* driver);
    int findDriver(const std::string& name) const;
    const PhysicsDriver* getDriver(int id) const;

    int packSize() const;
    void* pack(void*) const;
    void* unpack(void*);

    void print(std::ostream& out, const std::string& indent) const;

  private:
    typedef std::map<std::string, PhysicsDriver*> NameMap;
    struct PointerCompare {
      bool operator()(const PhysicsDriver* a, const PhysicsDriver* b) const {
        return *a < *b;
      }
    };
    typedef std::set<PhysicsDriver*, PointerCompare> DriverSet;
    typedef std::vector<PhysicsDriver*> DriverVec;

    DriverVec drivers;
    DriverSet driverSet;
    NameMap   nameMap;
};


inline const PhysicsDriver* PhysicsDriverManager::getDriver(int id) const
{
  if ((id >= 0) && (id < (int)drivers.size())) {
    return drivers[id];
  } else {
    return NULL;
  }
}


extern PhysicsDriverManager PHYDRVMGR;


#endif //_PHYSICS_DRIVER_H_

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
