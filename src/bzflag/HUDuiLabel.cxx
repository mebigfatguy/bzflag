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

// interface headers
#include "HUDuiControl.h"
#include "HUDuiLabel.h"

// system headers
#include <string>
#include <vector>

// common implementation headers
#include "common.h"
#include "BundleMgr.h"
#include "Bundle.h"
#include "FontManager.h"

// local implementation headers
#include "HUDui.h"

//
// HUDuiLabel
//

HUDuiLabel::HUDuiLabel() : HUDuiControl()
{
  darker = false;
  params = NULL;
  color[0] = textColor[0];
  color[1] = textColor[1];
  color[2] = textColor[2];
}

HUDuiLabel::~HUDuiLabel()
{
  if (params)
    delete params;
}

std::string		HUDuiLabel::getString() const
{
  std::string theString;
  Bundle *bdl = BundleMgr::getCurrentBundle();
  if (params)
    theString = bdl->formatMessage(string, params);
  else
    theString = bdl->getLocalString(string);

  return theString;
}

void			HUDuiLabel::setString(const std::string& _string, const std::vector<std::string> *_params)
{
  string = _string;
  if (_params) {
    if (params != NULL)
      delete params;

    params = new std::vector<std::string>(*_params);
  }
  onSetFont();
}

void			HUDuiLabel::onSetFont()
{
  HUDuiControl::onSetFont();
}

bool			HUDuiLabel::doKeyPress(const BzfKeyEvent& key)
{
  if (key.ascii == 0) switch (key.button) {
    case BzfKeyEvent::Up:
      HUDui::setFocus(getPrev());
      break;

    case BzfKeyEvent::Down:
      HUDui::setFocus(getNext());
      break;

    default:
      return false;
  }

  if (key.ascii == '\t') {
    HUDui::setFocus(getNext());
    return true;
  }

  switch (key.ascii) {
    case 13:
    case 27:
      return false;
  }
  return true;
}

bool			HUDuiLabel::doKeyRelease(const BzfKeyEvent&)
{
  return false;
}

void			HUDuiLabel::setDarker(bool d)
{
  darker = d;
}

void			HUDuiLabel::setColor(GLfloat r, GLfloat g, GLfloat b)
{
  color[0] = r;
  color[1] = g;
  color[2] = b;
}

void			HUDuiLabel::doRender()
{
  if (getFontFace() < 0) {
    return;
  }
  // render string
  GLfloat dimmedcolor[3] = {color[0]*0.7f, color[1]*0.7f, color[2]*0.7f};
  const float* myColor;
  if (hasFocus()) {
    myColor = color;
  } else if (!darker) {
    myColor = dimmedcolor;
  } else {
    myColor = moreDimTextColor;
  }
  FontManager &fm = FontManager::instance();
  fm.drawString(getX(), getY(), 0,
                getFontFace(), getFontSize(),
                getString(), myColor);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8

