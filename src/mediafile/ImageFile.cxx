/* bzflag
 * Copyright (c) 1993 - 2002 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named LICENSE that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include "ImageFile.h"

//
// ImageFile
//

ImageFile::ImageFile(istream* _stream) : MediaFile(_stream), open(false)
{
	// do nothing
}

ImageFile::~ImageFile()
{
	// do nothing
}

void					ImageFile::init(int _numChannels,
								int _width, int _height)
{
	open        = true;
	numChannels = _numChannels;
	width       = _width;
	height      = _height;
}

bool					ImageFile::isOpen() const
{
	return open;
}

int						ImageFile::getNumChannels() const
{
	return numChannels;
}

int						ImageFile::getWidth() const
{
	return width;
}

int						ImageFile::getHeight() const
{
	return height;
}
