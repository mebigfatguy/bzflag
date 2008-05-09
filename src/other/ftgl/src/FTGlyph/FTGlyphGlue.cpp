/*
 * FTGL - OpenGL font library
 *
 * Copyright (c) 2001-2004 Henry Maddocks <ftgl@opengl.geek.nz>
 *               2008 Sam Hocevar <sam@zoy.org>
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

#include "FTGL/ftgl.h"

#include "FTInternals.h"

static FTPoint static_ftpoint;
static FTBBox static_ftbbox;

FTGL_BEGIN_C_DECLS

#define C_TOR(cname, cargs, cxxname, cxxarg, cxxtype) \
    FTGLglyph* cname cargs \
    { \
        cxxname *g = new cxxname cxxarg; \
        if(g->Error()) \
        { \
            delete g; \
            return NULL; \
        } \
        FTGLglyph *ftgl = (FTGLglyph *)malloc(sizeof(FTGLglyph)); \
        ftgl->ptr = g; \
        ftgl->type = cxxtype; \
        return ftgl; \
    }

// FTBitmapGlyph::FTBitmapGlyph();
C_TOR(ftglCreateBitmapGlyph, (FT_GlyphSlot glyph),
      FTBitmapGlyph, (glyph), GLYPH_BITMAP);

// FTExtrudeGlyph::FTExtrudeGlyph();
C_TOR(ftglCreateExtrudeGlyph, (FT_GlyphSlot glyph, float depth,
                   float frontOutset, float backOutset, int useDisplayList),
      FTExtrudeGlyph, (glyph, depth, frontOutset, backOutset, (useDisplayList != 0)),
      GLYPH_EXTRUDE);

// FTOutlineGlyph::FTOutlineGlyph();
C_TOR(ftglCreateOutlineGlyph, (FT_GlyphSlot glyph, float outset,
                               int useDisplayList),
      FTOutlineGlyph, (glyph, outset, (useDisplayList != 0)), GLYPH_OUTLINE);

// FTPixmapGlyph::FTPixmapGlyph();
C_TOR(ftglCreatePixmapGlyph, (FT_GlyphSlot glyph),
      FTPixmapGlyph, (glyph), GLYPH_PIXMAP);

// FTPolygonGlyph::FTPolygonGlyph();
C_TOR(ftglCreatePolyGlyph, (FT_GlyphSlot glyph, float outset,
                            int useDisplayList),
      FTPolygonGlyph, (glyph, outset, (useDisplayList != 0)), GLYPH_OUTLINE);

// FTTextureGlyph::FTTextureGlyph();
C_TOR(ftglCreateTextureGlyph, (FT_GlyphSlot glyph, int id, int xOffset,
                               int yOffset, int width, int height),
      FTTextureGlyph, (glyph, id, xOffset, yOffset, width, height),
      GLYPH_TEXTURE);

#define C_FUN(cret, cname, cargs, cxxerr, cxxname, cxxarg) \
    cret cname cargs \
    { \
        if(!g || !g->ptr) \
        { \
            fprintf(stderr, "FTGL warning: NULL pointer in %s\n", #cname); \
            cxxerr; \
        } \
        switch(g->type) \
        { \
            case FTGL::GLYPH_BITMAP: \
                return dynamic_cast<FTBitmapGlyph*>(g->ptr)->cxxname cxxarg; \
            case FTGL::GLYPH_EXTRUDE: \
                return dynamic_cast<FTExtrudeGlyph*>(g->ptr)->cxxname cxxarg; \
            case FTGL::GLYPH_OUTLINE: \
                return dynamic_cast<FTOutlineGlyph*>(g->ptr)->cxxname cxxarg; \
            case FTGL::GLYPH_PIXMAP: \
                return dynamic_cast<FTPixmapGlyph*>(g->ptr)->cxxname cxxarg; \
            case FTGL::GLYPH_POLYGON: \
                return dynamic_cast<FTPolygonGlyph*>(g->ptr)->cxxname cxxarg; \
            case FTGL::GLYPH_TEXTURE: \
                return dynamic_cast<FTTextureGlyph*>(g->ptr)->cxxname cxxarg; \
        } \
        fprintf(stderr, "FTGL warning: %s not implemented for %d\n", #cname, g->type); \
        cxxerr; \
    }

// FTGlyph::~FTGlyph();
void ftglDestroyGlyph(FTGLglyph *g)
{
    if(!g || !g->ptr)
    {
        fprintf(stderr, "FTGL warning: NULL pointer in %s\n", __FUNCTION__);
        return;
    }
    switch(g->type)
    {
        case FTGL::GLYPH_BITMAP:
            delete dynamic_cast<FTBitmapGlyph*>(g->ptr); break;
        case FTGL::GLYPH_EXTRUDE:
            delete dynamic_cast<FTExtrudeGlyph*>(g->ptr); break;
        case FTGL::GLYPH_OUTLINE:
            delete dynamic_cast<FTOutlineGlyph*>(g->ptr); break;
        case FTGL::GLYPH_PIXMAP:
            delete dynamic_cast<FTPixmapGlyph*>(g->ptr); break;
        case FTGL::GLYPH_POLYGON:
            delete dynamic_cast<FTPolygonGlyph*>(g->ptr); break;
        case FTGL::GLYPH_TEXTURE:
            delete dynamic_cast<FTTextureGlyph*>(g->ptr); break;
        default:
            fprintf(stderr, "FTGL warning: %s not implemented for %d\n",
                            __FUNCTION__, g->type);
            break;
    }

    g->ptr = NULL;
    free(g);
}

// const FTPoint& FTGlyph::Render(const FTPoint& pen, int renderMode);
C_FUN(static const FTPoint&, _ftglRenderGlyph, (FTGLglyph *g,
                                   const FTPoint& pen, int renderMode),
      return static_ftpoint, Render, (pen, renderMode));

void ftglRenderGlyph(FTGLglyph *g, FTGL_DOUBLE penx, FTGL_DOUBLE peny,
                     int renderMode, FTGL_DOUBLE *advancex,
                     FTGL_DOUBLE *advancey)
{
    FTPoint pen(penx, peny);
    FTPoint ret = _ftglRenderGlyph(g, pen, renderMode);
    *advancex = ret.X();
    *advancey = ret.Y();
}

// const FTPoint& FTGlyph::Advance() const;
C_FUN(static const FTPoint&, _ftglGetGlyphAdvance, (FTGLglyph *g),
      return static_ftpoint, Advance, ());

void ftglGetGlyphAdvance(FTGLglyph *g, FTGL_DOUBLE *advancex,
                         FTGL_DOUBLE *advancey)
{
    FTPoint ret = _ftglGetGlyphAdvance(g);
    *advancex = ret.X();
    *advancey = ret.Y();
}

// const FTBBox& FTGlyph::BBox() const;
C_FUN(static const FTBBox&, _ftglGetGlyphBBox, (FTGLglyph *g),
      return static_ftbbox, BBox, ());

void ftglGetGlyphBBox(FTGLglyph *g, float bounds[6])
{
    FTBBox ret = _ftglGetGlyphBBox(g);
    FTPoint lower = ret.Lower(), upper = ret.Upper();
    bounds[0] = lower.Xf(); bounds[1] = lower.Yf(); bounds[2] = lower.Zf();
    bounds[3] = upper.Xf(); bounds[4] = upper.Yf(); bounds[5] = upper.Zf();
}

// FT_Error FTGlyph::Error() const;
C_FUN(FT_Error, ftglGetGlyphError, (FTGLglyph *g), return -1, Error, ());

FTGL_END_C_DECLS
