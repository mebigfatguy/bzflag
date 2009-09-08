/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2009 Sam Hocevar <sam@hocevar.net>
 *               2009 Mathew Eis (kingrobot)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "config.h"

#include "FTCleanup.h"

FTCleanup *FTCleanup::_instance = 0;


FTCleanup::FTCleanup()
{
}


FTCleanup::~FTCleanup()
{
    std::set<FT_Face **>::iterator cleanupItr = cleanupFT_FaceItems.begin();
    FT_Face **cleanupFace = 0;

    while (cleanupItr != cleanupFT_FaceItems.end())
    {
        cleanupFace = *cleanupItr;
        if (*cleanupFace)
        {
            FT_Done_Face(**cleanupFace);
            delete *cleanupFace;
            *cleanupFace = 0;
        }
        cleanupItr++;
    }
    cleanupFT_FaceItems.clear();
}


void FTCleanup::RegisterObject(FT_Face **obj)
{
    cleanupFT_FaceItems.insert(obj);
}


void FTCleanup::UnregisterObject(FT_Face **obj)
{
    cleanupFT_FaceItems.erase(obj);
}

