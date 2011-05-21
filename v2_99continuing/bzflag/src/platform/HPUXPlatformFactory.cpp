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

#include "HPUXPlatformFactory.h"
#include "XDisplay.h"
#include "XVisual.h"
#include "XWindow.h"
#include "HPUXMedia.h"

PlatformFactory*  PlatformFactory::getInstance() {
  if (!instance) { instance = new HPUXPlatformFactory; }
  return instance;
}

HPUXPlatformFactory::HPUXPlatformFactory() {
  // do nothing
}

HPUXPlatformFactory::~HPUXPlatformFactory() {
  // do nothing
}

BzfDisplay*   HPUXPlatformFactory::createDisplay(
  const char* name, const char*) {
  XDisplay* display = new XDisplay(name);
  if (!display || !display->isValid()) {
    delete display;
    return NULL;
  }
  return display;
}

BzfVisual*    HPUXPlatformFactory::createVisual(
  const BzfDisplay* display) {
  return new XVisual((const XDisplay*)display);
}

BzfWindow*    HPUXPlatformFactory::createWindow(
  const BzfDisplay* display, BzfVisual* visual) {
  return new XWindow((const XDisplay*)display, (XVisual*)visual);
}

BzfMedia*   HPUXPlatformFactory::createMedia() {
  return new HPUXMedia;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8
