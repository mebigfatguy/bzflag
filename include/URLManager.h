/* bzflag
 * Copyright (c) 1993 - 2005 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * abstracted URL class
 */

#ifndef URL_MANAGER_H
#define URL_MANAGER_H

// bzflag common header
#include "common.h"

// system headers
#include <string>
#include <map>
#include <vector>
#include "time.h"

// local implementation headers
#include "Singleton.h"

class URLManager : public Singleton<URLManager> {
  public:
    bool getURL(const std::string& URL, std::string &data);
    bool getURL(const std::string& URL, void **data, unsigned int& size);
    bool getURLHeader(const std::string& URL);

    bool getFileTime(time_t &t);
    const char* getErrorString() const;

    void setProgressFunc(int (*func)(void* clientp,
				     double dltotal, double dlnow,
				     double ultotal, double ulnow),
				     void* data);

    void freeURLData(void *data);

    void collectData(char* ptr, int len);

  protected:
    friend class Singleton<URLManager>;
    URLManager();
    ~URLManager();

  private:
    void clearInternal();
    bool beginGet(const std::string URL);

  private:
    void *easyHandle; // this is CURL specific (used as 'CURL*')

    int errorCode;
    void *theData;
    unsigned int theLen;
};


#endif // URL_MANAGER_H

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

