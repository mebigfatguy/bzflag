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

#ifdef _MSC_VER
#pragma warning( 4: 4786)
#endif

#include "motd.h"
#include "URLManager.h"
#include "TextUtils.h"

MessageOfTheDay::MessageOfTheDay()
{
}

MessageOfTheDay::~MessageOfTheDay()
{

}

const std::string& MessageOfTheDay::get ( const std::string URL )
{
  data = "";
	return data;
  // get all up on the internet and go get the thing
  if (!URLManager::instance().getURL(URL,data)) {
    data = "Default MOTD";
  } else {
    // trim trailing whitespace
    int l = data.size() - 1;
    while (TextUtils::isWhitespace(data[l])) {
      data.erase(l, 1);
      l--;
    }
  }

  return data;
}
