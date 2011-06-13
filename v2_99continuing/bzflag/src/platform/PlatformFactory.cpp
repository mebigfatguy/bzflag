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

#include "PlatformFactory.h"
#include "bzfSDL.h"
#include "common/ErrorHandler.h"
#include "platform/BzfJoystick.h"
#include "platform/BzfMedia.h"

PlatformFactory*  PlatformFactory::instance = 0;
BzfMedia*   PlatformFactory::media = 0;

PlatformFactory::PlatformFactory() {
#ifdef HAVE_SDL
  Uint32 flags = 0;
#ifdef DEBUG
  flags |= SDL_INIT_NOPARACHUTE;
#endif
  if (SDL_Init(flags) == -1) {
    printFatalError("Could not initialize SDL: %s.\n", SDL_GetError());
    exit(-1);
  };
#endif
}

PlatformFactory::~PlatformFactory() {
#ifdef HAVE_SDL
  if (media) {
    media->closeAudio();
  }
  SDL_Quit();
#endif
  delete media;
}

BzfJoystick*    PlatformFactory::createJoystick() {
  // if a platform doesn't have a native joystick impl., bzfjoystick provides defaults.
  return new BzfJoystick();
}

BzfMedia*   PlatformFactory::getMedia() {
  if (!media) { media = getInstance()->createMedia(); }
  return media;
}

// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
