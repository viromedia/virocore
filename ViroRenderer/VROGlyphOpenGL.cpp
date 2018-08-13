//
//  VROGlyphOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROGlyphOpenGL.h"
#include "VROLog.h"
#include "VROTexture.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROMath.h"
#include "VRODriverOpenGL.h"
#include "VRODefines.h"
#include "VROVectorizer.h"
#include "VROContour.h"
#include "VROGlyphAtlasOpenGL.h"
#include "poly2tri/poly2tri.h"

#if VRO_PLATFORM_WASM
#include "ftstroke.h"
#else
#include "freetype/ftstroke.h"
#endif

static const int kBezierSteps = 4;
static const float kExtrusion = 1;

VROGlyphOpenGL::VROGlyphOpenGL() {
    
}

VROGlyphOpenGL::~VROGlyphOpenGL() {
    
}

bool VROGlyphOpenGL::loadGlyph(FT_Face face, uint32_t charCode, uint32_t variantSelector) {
    /*
     Load the glyph from freetype.
     */
    if (variantSelector != 0) {
        FT_UInt glyphIndex = FT_Face_GetCharVariantIndex(face, charCode, variantSelector);
        if (glyphIndex == 0) {
            // Undefined character code, just attempt to load without the selector
            if (FT_Load_Char(face, charCode, FT_LOAD_DEFAULT)) {
                pinfo("Failed to load glyph %d (dropped variant selector %d)", charCode, variantSelector);
                return false;
            }
        }
        else {
            if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT)) {
                pinfo("Failed to load glyph %d with variant selector %d", charCode, variantSelector);
                return false;
            }
        }
    }
    else if (FT_Load_Char(face, charCode, FT_LOAD_DEFAULT)) {
        pinfo("Failed to load glyph %d", charCode);
        return false;
    }
    
    FT_GlyphSlot &glyph = face->glyph;
    
    /*
     Each advance unit is 1/64 of a pixel so divide by 64 (>> 6) to get advance in pixels.
     */
    _advance = glyph->advance.x >> 6;
    return true;
}

bool VROGlyphOpenGL::loadMetrics(FT_Face face, uint32_t charCode, uint32_t variantSelector) {
    return loadGlyph(face, charCode, variantSelector);
}

bool VROGlyphOpenGL::loadBitmap(FT_Face face, uint32_t charCode, uint32_t variantSelector,
                                std::vector<std::shared_ptr<VROGlyphAtlas>> *glyphAtlases,
                                std::shared_ptr<VRODriver> driver) {
    if (!loadGlyph(face, charCode, variantSelector)) {
        return false;
    }
    FT_GlyphSlot &glyph = face->glyph;
    
    if (glyphAtlases->empty()) {
        glyphAtlases->push_back(std::make_shared<VROGlyphAtlasOpenGL>(false));
    }
    std::shared_ptr<VROGlyphAtlas> atlas = glyphAtlases->back();
    
    FT_Render_Glyph(glyph, FT_RENDER_MODE_LIGHT);
    FT_Bitmap &bitmap = glyph->bitmap;
    
    VROAtlasLocation location;
    if (atlas->glyphWillFit(bitmap, &location)) {
        atlas->write(bitmap, location, driver);
    } else {
        // Did not fit in the atlas, create a new atlas
        glyphAtlases->push_back(std::make_shared<VROGlyphAtlasOpenGL>(false));
        atlas = glyphAtlases->back();
        
        if (atlas->glyphWillFit(bitmap, &location)) {
            atlas->write(bitmap, location, driver);
        } else {
            pinfo("Failed to render glyph for char code %d", charCode);
            return false;
        }
    }
    
    VROGlyphBitmap vBitmap;
    vBitmap.atlas = atlas;
    vBitmap.bearing = VROVector3f(glyph->bitmap_left, glyph->bitmap_top);
    vBitmap.size = VROVector3f(bitmap.width, bitmap.rows);
    vBitmap.minU = ((float) location.minU) / (float) atlas->getSize();
    vBitmap.maxU = ((float) location.maxU) / (float) atlas->getSize();
    vBitmap.minV = ((float) location.minV) / (float) atlas->getSize();
    vBitmap.maxV = ((float) location.maxV) / (float) atlas->getSize();
    
    _bitmaps[0] = vBitmap;
    return true;
}

bool VROGlyphOpenGL::loadOutlineBitmap(FT_Library library, FT_Face face, uint32_t charCode, uint32_t variantSelector,
                                       uint32_t outlineWidth,
                                       std::vector<std::shared_ptr<VROGlyphAtlas>> *glyphAtlases,
                                       std::shared_ptr<VRODriver> driver) {
    if (!loadGlyph(face, charCode, variantSelector)) {
        return false;
    }
    
    FT_GlyphSlot &glyphSlot = face->glyph;
    if (glyphAtlases->empty()) {
        glyphAtlases->push_back(std::make_shared<VROGlyphAtlasOpenGL>(true));
    }
    std::shared_ptr<VROGlyphAtlas> atlas = glyphAtlases->back();
    
    FT_Stroker stroker;
    FT_Stroker_New(library, &stroker);
    FT_Stroker_Set(stroker, outlineWidth * 64, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);

    FT_Glyph glyph;
    FT_Get_Glyph(glyphSlot, &glyph);
    FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
    FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
    FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
    FT_Bitmap &bitmap = bitmapGlyph->bitmap;
    
    VROAtlasLocation location;
    if (atlas->glyphWillFit(bitmap, &location)) {
        atlas->write(bitmap, location, driver);
    } else {
        // Did not fit in the atlas, create a new atlas
        glyphAtlases->push_back(std::make_shared<VROGlyphAtlasOpenGL>(true));
        atlas = glyphAtlases->back();
        
        if (atlas->glyphWillFit(bitmap, &location)) {
            atlas->write(bitmap, location, driver);
        } else {
            pinfo("Failed to render glyph for char code %d", charCode);
            return false;
        }
    }
    
    VROGlyphBitmap vBitmap;
    vBitmap.atlas = atlas;
    vBitmap.bearing = VROVector3f(glyphSlot->bitmap_left, glyphSlot->bitmap_top);
    vBitmap.size = VROVector3f(bitmap.width, bitmap.rows);
    vBitmap.minU = ((float) location.minU) / (float) atlas->getSize();
    vBitmap.maxU = ((float) location.maxU) / (float) atlas->getSize();
    vBitmap.minV = ((float) location.minV) / (float) atlas->getSize();
    vBitmap.maxV = ((float) location.maxV) / (float) atlas->getSize();
    
    _bitmaps[(int) outlineWidth] = vBitmap;
    return true;
}

bool VROGlyphOpenGL::loadVector(FT_Face face, uint32_t charCode, uint32_t variantSelector) {
    if (!loadGlyph(face, charCode, variantSelector)) {
        return false;
    }
    
    FT_GlyphSlot &glyph = face->glyph;
    if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) {
        pwarn("Outline glyph format required to vectorize, aborting 3D font building");
        return false;
    }
    
    VROVectorizer vectorizer(glyph, kBezierSteps);
    
    /*
     Contour the sides.
     */
    for (size_t ci = 0; ci < vectorizer.getContourCount(); ++ci) {
        const VROContour *contour = vectorizer.getContour(ci);
        for (size_t p = 0; p < contour->getPointCount() - 1; ++p) {
            const VROVector3f d1 = contour->getPoint(p);
            const VROVector3f d2 = contour->getPoint(p + 1);
            
            VROVector3f a, b, c;
            a.x = (d1.x / 64.0f);
            a.y = d1.y / 64.0f;
            a.z = 0.0f;
            b.x = (d2.x / 64.0f);
            b.y = d2.y / 64.0f;
            b.z = 0.0f;
            c.x = (d1.x / 64.0f);
            c.y = d1.y / 64.0f;
            c.z = kExtrusion;
            
            VROGlyphTriangle t1(a, b, c, VROGlyphTriangleType::Side);
            _triangles.push_back(t1);
            
            a.x = (d1.x / 64.0f);
            a.y = d1.y / 64.0f;
            a.z = kExtrusion;
            b.x = (d2.x / 64.0f);
            b.y = d2.y / 64.0f;
            b.z = kExtrusion;
            c.x = (d2.x / 64.0f);
            c.y = d2.y / 64.0f;
            c.z = 0.0f;
            
            VROGlyphTriangle t2(a, b, c, VROGlyphTriangleType::Side);
            _triangles.push_back(t2);
        }
        
        /*
         Add the last triangle closing the contour of the sides.
         */
        const VROVector3f d1 = contour->getPoint(contour->getPointCount() - 1);
        const VROVector3f d2 = contour->getPoint(0);
        VROVector3f a, b, c;
        a.x = (d1.x / 64.0f);
        a.y = d1.y / 64.0f;
        a.z = 0.0f;
        b.x = (d2.x / 64.0f);
        b.y = d2.y / 64.0f;
        b.z = 0.0f;
        c.x = (d1.x / 64.0f);
        c.y = d1.y / 64.0f;
        c.z = kExtrusion;
        
        VROGlyphTriangle t1(a, b, c, VROGlyphTriangleType::Side);
        _triangles.push_back(t1);
        
        a.x = (d1.x / 64.0f);
        a.y = d1.y / 64.0f;
        a.z = kExtrusion;
        b.x = (d2.x / 64.0f);
        b.y = d2.y / 64.0f;
        b.z = kExtrusion;
        c.x = (d2.x / 64.0f);
        c.y = d2.y / 64.0f;
        c.z = 0.0f;
        
        VROGlyphTriangle t2(a, b, c, VROGlyphTriangleType::Side);
        _triangles.push_back(t2);
        
        if (contour->getDirection()) {
            try {
                std::vector<p2t::Point *> polyline = triangulateContour(vectorizer, (int) ci);
                p2t::CDT *cdt = new p2t::CDT(polyline);

                for (size_t cm = 0; cm < vectorizer.getContourCount(); ++cm) {
                    const VROContour *sm = vectorizer.getContour(cm);
                    if (ci != cm && !sm->getDirection() && sm->isInside(contour)) {
                        std::vector<p2t::Point *> pl = triangulateContour(vectorizer, (int) cm);
                        cdt->AddHole(pl);
                    }
                }

                cdt->Triangulate();
                std::vector<p2t::Triangle *> ts = cdt->GetTriangles();
                for (int i = 0; i < ts.size(); i++) {
                    p2t::Triangle *ot = ts[i];

                    VROVector3f a, b, c;
                    a.x = ot->GetPoint(0)->x;
                    a.y = ot->GetPoint(0)->y;
                    a.z = 0.0f;
                    b.x = ot->GetPoint(1)->x;
                    b.y = ot->GetPoint(1)->y;
                    b.z = 0.0f;
                    c.x = ot->GetPoint(2)->x;
                    c.y = ot->GetPoint(2)->y;
                    c.z = 0.0f;

                    VROGlyphTriangle t1(a, b, c, VROGlyphTriangleType::Back);
                    _triangles.push_back(t1);

                    a.x = ot->GetPoint(0)->x;
                    a.y = ot->GetPoint(0)->y;
                    a.z = kExtrusion;
                    b.x = ot->GetPoint(1)->x;
                    b.y = ot->GetPoint(1)->y;
                    b.z = kExtrusion;
                    c.x = ot->GetPoint(2)->x;
                    c.y = ot->GetPoint(2)->y;
                    c.z = kExtrusion;

                    VROGlyphTriangle t2(a, b, c, VROGlyphTriangleType::Front);
                    _triangles.push_back(t2);
                }
                delete (cdt);
            } catch (const std::exception &e) {
                pwarn("Exception while triangulating text; text may be malformed");
            }
        }
    }
    return true;
}

std::vector<p2t::Point *> VROGlyphOpenGL::triangulateContour(VROVectorizer &vectorizer, int c) {
    std::vector<p2t::Point*> polyline;
    const VROContour *contour = vectorizer.getContour(c);
    for(size_t p = 0; p < contour->getPointCount(); ++p) {
        VROVector3f d = contour->getPoint(p);
        polyline.push_back(new p2t::Point((d.x / 64.0f), d.y / 64.0f));
    }
    return polyline;
}
