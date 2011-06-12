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

#ifndef __ENTRYZONES_H__
#define __ENTRYZONES_H__

// common header
#include "common.h"

// common headers
#include "game/Flag.h"
#include "vectors.h"

// bzfs headers
#include "CustomZone.h"


typedef std::vector<CustomZone> ZoneList;
typedef std::vector<std::pair<int, float> > QPairList;
typedef std::map<std::string, QPairList> QualifierMap;


class EntryZones {
  public:
    EntryZones();

    void addZone(const CustomZone* zone);
    void addZoneFlag(int zone, int flagId);

    void calculateQualifierLists();

    bool getZonePoint(const std::string& qualifier, fvec3& pt) const;
    bool getSafetyPoint(const std::string& qualifier,
                        const fvec3& pos, fvec3& pt) const;

    bool getRandomPoint(const std::string& qual, fvec3& pt) const;
    bool getClosePoint(const std::string& qual, const fvec3& pos,
                       fvec3& pt) const;

    const ZoneList& getZoneList() const;

    int packSize() const;
    void* pack(void* buf) const;

  private:
    ZoneList zones;
    QualifierMap qmap;

    void makeSplitLists(int zone,
                        std::vector<FlagType*> &flags,
                        std::vector<TeamColor> &teams,
                        std::vector<TeamColor> &safety) const;
};

#endif

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
