/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/* interface header */
#include "Downloads.h"

/* common implementation headers */
#include "network.h"
#include "Address.h"
#include "AccessList.h"
#include "CacheManager.h"
#include "BzMaterial.h"
#include "URLManager.h"
#include "AnsiCodes.h"
#include "TextureManager.h"

/* local implementation headers */
#include "playing.h"


// FIXME - someone write a better explanation
static const char DownloadContent[] =
  "#\n"
  "# This file controls the access to servers for downloads.\n"
  "# Patterns are attempted in order against both the hostname\n"
  "# and ip. The first matching pattern sets the state. If no\n"
  "# patterns are matched, then the server is authorized. There\n"
  "# are four types of matches:\n"
  "#\n"
  "#   simple globbing (* and ?)\n"
  "#     allow\n"
  "#     deny\n"
  "#\n"
  "#   regular expressions\n"
  "#     allow_regex\n"
  "#     deny_regex\n"
  "#\n"
  "\n"
  "#\n"
  "# To authorize all servers, remove the last 3 lines.\n"
  "#\n"
  "\n"
  "allow *.bzflag.bz\n"
  "allow *.bzflag.org\n"
  "deny *\n";
  
static AccessList DownloadAccessList("DownloadAccess.txt", DownloadContent);


/******************************************************************************/
#ifdef HAVE_CURL
/******************************************************************************/


// Function Prototypes
static void printAuthNotice();
static bool getFileTime(const std::string& url, time_t& t);
static bool getAndCacheURL(const std::string& url);
static bool authorizedServer(const std::string& url);


void Downloads::doDownloads()
{
  CACHEMGR.loadIndex();
  CACHEMGR.limitCacheSize();

  DownloadAccessList.reload();

  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set, false /* ignore referencing */); 
  
  bool authNotice = false;

  const bool doDownloads =	BZDB.isTrue("doDownloads");
  const bool updateDownloads =  BZDB.isTrue("updateDownloads");

  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFileType(texUrl)) {

      // check access authorization
      if (!authorizedServer(texUrl)) {
        MATERIALMGR.setTextureLocal(texUrl, "");
        authNotice = true;
        continue;
      }

      // use the cache?
      CacheManager::CacheRecord oldrec;
      if (CACHEMGR.findURL(texUrl, oldrec)) {
        time_t filetime = 0;
        if (doDownloads && updateDownloads) {
          getFileTime(texUrl, filetime);
        }
        if (filetime <= oldrec.date) {
          // use the cached file
          MATERIALMGR.setTextureLocal(texUrl, oldrec.name);
          continue;
        }
      }
      
      // bail here if we can't download
      if (!doDownloads) {
        MATERIALMGR.setTextureLocal(texUrl, "");
        std::string msg = ColorStrings[GreyColor];
        msg += "not downloading: " + texUrl;
        addMessage(NULL, msg);
        continue;
      }
      
      // download and cache the URL
      if (getAndCacheURL(texUrl)) {
        const std::string localname = CACHEMGR.getLocalName(texUrl);
        MATERIALMGR.setTextureLocal(texUrl, localname);
      } else {
        MATERIALMGR.setTextureLocal(texUrl, "");
      }
    }
  }
  
  if (authNotice) {
    printAuthNotice();
  }

  CACHEMGR.saveIndex();

  return;
}


bool Downloads::updateDownloads(bool& rebuild)
{
  CACHEMGR.loadIndex();
  CACHEMGR.limitCacheSize();
  
  DownloadAccessList.reload();
  
  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set, true /* only referenced materials */);
  
  TextureManager& TEXMGR = TextureManager::instance();

  rebuild = false;
  bool updated = false;
  bool authNotice = false;
  
  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFileType(texUrl)) {

      // check access authorization
      if (!authorizedServer(texUrl)) {
        MATERIALMGR.setTextureLocal(texUrl, "");
        authNotice = true;
        continue;
      }

      // use the cache or update?
      CacheManager::CacheRecord oldrec;
      if (CACHEMGR.findURL(texUrl, oldrec)) {
        time_t filetime;
        getFileTime(texUrl, filetime);
        if (filetime <= oldrec.date) {
          // keep using the cached file
          MATERIALMGR.setTextureLocal(texUrl, oldrec.name);
          if (!TEXMGR.isLoaded(oldrec.name)) {
            rebuild = true;
          }
          continue;
        }
      }

      // download the file and update the cache
      if (getAndCacheURL(texUrl)) {
        updated = true;
        const std::string localname = CACHEMGR.getLocalName(texUrl);
        if (!TEXMGR.isLoaded(localname)) {
          rebuild = true; 
        } else {
          TEXMGR.reloadTextureImage(localname); // reload with the new image
        }
        MATERIALMGR.setTextureLocal(texUrl, localname); // if it wasn't cached
      }
    }
  }
  
  if (authNotice) {
    printAuthNotice();
  }

  CACHEMGR.saveIndex();
  
  return updated;
}


void Downloads::removeTextures()
{
  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set, false /* ignore referencing */);
  
  TextureManager& TEXMGR = TextureManager::instance();

  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFileType(texUrl)) {
      const std::string& localname = CACHEMGR.getLocalName(texUrl);
      if (TEXMGR.isLoaded(localname)) {
        TEXMGR.removeTexture(localname);
      }
    }
  }
  
  return;
}


static void printAuthNotice()
{
  std::string msg = ColorStrings[WhiteColor];
  msg += "NOTE: ";
  msg += ColorStrings[GreyColor];
  msg += "download access is controlled by ";
  msg += ColorStrings[YellowColor];
  msg += DownloadAccessList.getFileName();
  addMessage(NULL, msg);
  return;
}


static bool getFileTime(const std::string& url, time_t& t)
{
  URLManager& URLMGR = URLManager::instance();
  if (URLMGR.getURLHeader(url)) {
    URLMGR.getFileTime(t);
    return true;
  } else {
    t = 0;
    return false;
  }
}


static bool getAndCacheURL(const std::string& url)
{
  bool result = false;
  
  URLManager& URLMGR = URLManager::instance();
  URLMGR.setProgressFunc(curlProgressFunc, NULL);

  std::string msg = ColorStrings[GreyColor];
  msg += "downloading: " + url;
  addMessage(NULL, msg);
  
  void* urlData;
  unsigned int urlSize;

  if (URLMGR.getURL(url, &urlData, urlSize)) {
    time_t filetime;
    URLMGR.getFileTime(filetime);
    // CACHEMGR generates name, usedDate, and key
    CacheManager::CacheRecord rec;
    rec.url = url;
    rec.size = urlSize;
    rec.date = filetime;
    CACHEMGR.addFile(rec, urlData);
    free(urlData);
    result = true;
  }
  else {
    std::string msg = ColorStrings[RedColor] + "* failure *";
    addMessage(NULL, msg);
    result = false;
  }

  URLMGR.setProgressFunc(NULL, NULL);

  return result;
}


static bool authorizedServer(const std::string& url)
{
  // avoid the DNS lookup
  if (DownloadAccessList.alwaysAuthorized()) {
    return true;
  }
  
  // parse url
  std::string protocol, hostname, path, ip;
  int port = 1;
  if (BzfNetwork::parseURL(url, protocol, hostname, port, path) &&
      ((protocol == "http") || (protocol == "ftp")) &&
      (port >= 1) && (port <= 65535)) {
    Address address(hostname); // get the address  (BLOCKING)
    ip = address.getDotNotation();
  }

  // make the list of strings to check  
  std::vector<std::string> nameAndIp;
  if (hostname.size() > 0) {
    nameAndIp.push_back(hostname);
  }
  if (ip.size() > 0) {
    nameAndIp.push_back(ip);
  }

  // check and print error if not authorized
  if (!DownloadAccessList.authorized(nameAndIp)) {
    std::string msg = ColorStrings[RedColor];
    msg += "local server denial: ";
    msg += ColorStrings[GreyColor];
    msg += url;
    addMessage(NULL, msg);
    return false;
  }
  
  return true;
}


/******************************************************************************/
#else // ! HAVE_CURL
/******************************************************************************/


void Downloads::doDownloads()
{
  bool needWarning = false;
  BzMaterialManager::TextureSet set;
  BzMaterialManager::TextureSet::iterator set_it;
  MATERIALMGR.makeTextureList(set);

  for (set_it = set.begin(); set_it != set.end(); set_it++) {
    const std::string& texUrl = set_it->c_str();
    if (CACHEMGR.isCacheFileType(texUrl)) {
      needWarning = true;
      // one time warning
      std::string msg = ColorStrings[GreyColor];
      msg += "not downloading: " + texUrl;
      addMessage(NULL, msg);
      // avoid future warnings
      MATERIALMGR.setTextureLocal(texUrl, "");
    }
  }
  
  if (needWarning && BZDB.isTrue("doDownloads")) {
    std::string msg = ColorStrings[RedColor];
    msg += "Downloads are not available for clients without libcurl";
    addMessage(NULL, msg);
    msg = ColorStrings[YellowColor];
    msg += "To disable this message, disable [Automatic Downloads] in";
    addMessage(NULL, msg);
    msg = ColorStrings[YellowColor];
    msg += "Options -> Cache Options, or get a client with libcurl";
    addMessage(NULL, msg);
  }

  return;
}


bool Downloads::updateDownloads()
{
  std::string msg = ColorStrings[RedColor];
  msg += "Downloads are not available for clients without libcurl";
  addMessage(NULL, msg);
  return false;
}


/******************************************************************************/
#endif // HAVE_CURL
/******************************************************************************/


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
