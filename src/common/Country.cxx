/* bzflag
 * Copyright (c) 1993 - 2003 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "Country.h"

/* private */

/* protected */
bool Country::isValid(int country)
{
  return false;
}
bool Country::isValid(std::string country)
{
  return false;
}

/* public: */

Country::Country(std::string)
{
}
Country::~Country()
{
}

int Country::number() const
{
  return 0;
}
std::string Country::iso2() const
{
  return "";
}
std::string Country::iso3() const
{
  return "";
}
std::string Country::englishName() const
{
  return "";
}
std::string Country::frenchName() const
{
  return "";
}


int Country::number(int country)
{
  // XXX - validate number
  return country;
}
int Country::number(std::string country)
{
  return 0;
}
std::string Country::iso2(int country)
{
  return "";
}
std::string Country::iso2(std::string country)
{
  return "";
}
std::string Country::iso3(int country)
{
  return "";
}
std::string Country::iso3(std::string country)
{
  return "";
}
std::string Country::englishName(int country)
{
  return "";
}
std::string Country::englishName(std::string country)
{
  return "";
}
std::string Country::frenchName(int country)
{
  return "";
}
std::string Country::frenchName(std::string coutnry)
{
  return "";
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

