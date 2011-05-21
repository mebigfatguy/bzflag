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

/* interface header */
#include "ScriptLoaderFactory.h"

/* local implementation headers */
#include "SharedObjectScript.h"

#include "LuaScript.h"

#ifdef WITH_PYTHONLOADER
#   include "PythonScript.h"
#endif


// initialize the singleton
template <>
ScriptLoaderFactory* Singleton<ScriptLoaderFactory>::_instance = NULL;

/* public */

RobotScript*
ScriptLoaderFactory::scriptTool(std::string extension) {
  std::string lcExtension = TextUtils::tolower(extension);
  return SCRIPTTOOLFACTORY.Create(lcExtension.c_str());
}

void ScriptLoaderFactory::initialize() {
#if defined(_WIN32)
  SCRIPTTOOLFACTORY.Register<SharedObjectLoader>("dll");
#else
  SCRIPTTOOLFACTORY.Register<SharedObjectLoader>("so");
#endif /* defined(_WIN32) */

  SCRIPTTOOLFACTORY.Register<LuaScript>("lua");

#ifdef WITH_PYTHONLOADER
  SCRIPTTOOLFACTORY.Register<PythonLoader>("py");
#endif
}


/* private */
ScriptLoaderFactory::ScriptLoaderFactory() {
}


ScriptLoaderFactory::~ScriptLoaderFactory() {
}


// Local Variables: ***
// mode: C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: nil ***
// End: ***
// ex: shiftwidth=2 tabstop=8 expandtab
