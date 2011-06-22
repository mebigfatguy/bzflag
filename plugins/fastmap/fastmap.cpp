/* bzflag
 * Copyright (c) 1993-2011 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// httpTest.cpp : Defines the entry point for the DLL application.
//

#include <map>
#include <string>
#include <sstream>
#include "bzfsAPI.h"
#include "bzfsHTTPAPI.h"

class Fastmap : public bzhttp_VDir, public bz_Plugin
{
public:
  Fastmap(): bzhttp_VDir(),bz_Plugin(), mapData(NULL), mapDataSize(0) {}
  virtual ~Fastmap()
  {
    Unloadable = false;

    if (mapData)
      free(mapData);
    mapData = NULL;
  };
  const char* Name () { return "Fast Map";}

  virtual const char* VDirName(){return "fastmap";}
  virtual const char* VDirDescription(){return "Deploys maps over HTTP";}

  void Init(const char* /*commandLine*/)
  {
    bz_debugMessage(4,"Fastmap plugin loaded");
    Register(bz_eWorldFinalized);
    bzhttp_RegisterVDir(this,this);
  }

  void Cleanup(void)
  {
    Flush();
    bzhttp_RemoveAllVdirs(this);
  }

  virtual bzhttp_ePageGenStatus GeneratePage ( const bzhttp_Request& request, bzhttp_Responce &responce )
  {
    responce.ReturnCode = e200OK;
    responce.DocumentType = eOctetStream;

    if (mapData && mapDataSize) {
      responce.MD5Hash = md5;
      responce.AddBodyData(mapData,mapDataSize);
    } else {
      responce.AddBodyData("404 Fastmap not Valid");
      responce.ReturnCode = e404NotFound;
    }

    return ePageDone;
  }

  virtual void Event(bz_EventData * eventData)
  {
    if (eventData->eventType == bz_eWorldFinalized) {
      if (mapData)
	free(mapData);

      mapData = NULL;
      mapDataSize = 0;

      if (!bz_getPublic() || bz_getClientWorldDownloadURL().size())
	return;

      mapDataSize = bz_getWorldCacheSize();
      if (!mapDataSize)
	return;

      mapData = (char *) malloc(mapDataSize);
      if (!mapData) {
	mapDataSize = 0;
	return;
      }

      bz_getWorldCacheData((unsigned char*)mapData);

      md5 = bz_MD5(mapData,mapDataSize);

      std::string URL = BaseURL.c_str();
      bz_debugMessagef(2, "FastMap: Running local HTTP server for maps using URL %s", URL.c_str());
      bz_setClientWorldDownloadURL(URL.c_str());
    }
  }
  char *mapData;
  size_t mapDataSize;
  std::string md5;
};

BZ_PLUGIN(Fastmap)


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
