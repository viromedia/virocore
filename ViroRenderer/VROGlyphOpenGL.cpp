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
#include "VROGlyphAtlasOpenGL.h"

VROGlyphOpenGL::VROGlyphOpenGL() {
    
}

VROGlyphOpenGL::~VROGlyphOpenGL() {
    
}

bool VROGlyphOpenGL::load(FT_Face face, uint32_t charCode, uint32_t variantSelector,
                          bool forRendering, std::vector<std::shared_ptr<VROGlyphAtlas>> *glyphAtlases,
                          std::shared_ptr<VRODriver> driver) {
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
    
    if (forRendering) {
        if (glyphAtlases->empty()) {
            glyphAtlases->push_back(std::make_shared<VROGlyphAtlasOpenGL>());
        }
        std::shared_ptr<VROGlyphAtlas> atlas = glyphAtlases->back();
        
        FT_Render_Glyph(glyph, FT_RENDER_MODE_LIGHT);
        FT_Bitmap &bitmap = glyph->bitmap;
        
        VROAtlasLocation location;
        if (atlas->glyphWillFit(bitmap, &location)) {
            atlas->write(glyph, bitmap, location, driver);
        } else {
            // Did not fit in the atlas, create a new atlas
            glyphAtlases->push_back(std::make_shared<VROGlyphAtlasOpenGL>());
            atlas = glyphAtlases->back();
            
            if (atlas->glyphWillFit(bitmap, &location)) {
                atlas->write(glyph, bitmap, location, driver);
            } else {
                pinfo("Failed to render glyph for char code %d", charCode);
                return false;
            }
        }
        
        _atlas = atlas;
        _bearing = VROVector3f(glyph->bitmap_left, glyph->bitmap_top);
        _size = VROVector3f(bitmap.width, bitmap.rows);
        _minU = ((float) location.minU) / (float) atlas->getSize();
        _maxU = ((float) location.maxU) / (float) atlas->getSize();
        _minV = ((float) location.minV) / (float) atlas->getSize();
        _maxV = ((float) location.maxV) / (float) atlas->getSize();        
    }
    
    return true;
}

std::shared_ptr<VROTexture> VROGlyphOpenGL::getTexture() const {
    return _atlas->getTexture();
}
