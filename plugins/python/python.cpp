in/* bzflag
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

#include "bzfsAPI.h"
#include <Python.h>
#include "PyBZFlag.h"

Python::BZFlag *module_bzflag;

BZ_GET_PLUGIN_VERSION

BZF_PLUGIN_CALL
int
bz_Load (const char *commandLine)
{
  Py_SetProgramName ("blender");
  Py_Initialize ();

  module_bzflag = new Python::BZFlag ();
}

BZF_PLUGIN_CALL
int
bz_Unload (void)
{
  Py_Finalize ();
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
