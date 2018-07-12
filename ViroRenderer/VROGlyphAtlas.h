//
//  VROGlyphAtlas.h
//  ViroRenderer
//
//  Created by Raj Advani on 7/10/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROGlyphAtlas_h
#define VROGlyphAtlas_h

#include <memory>
#include "VROVector3f.h"
#include "VROAllocationTracker.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class VRODriver;
class VROTexture;

class VROAtlasLocation {
public:
    int minU, maxU, minV, maxV;
};

class VROGlyphAtlas {
    
public:
    
    VROGlyphAtlas() :
        _occupiedU(0),
        _occupiedBottomV(0),
        _occupiedTopV(0) {
        ALLOCATION_TRACKER_ADD(GlyphAtlases, 1);
    }
    virtual ~VROGlyphAtlas() {
        ALLOCATION_TRACKER_SUB(GlyphAtlases, 1);
    }
    
    /*
     Returns true if the given glyph will fit in this atlas. If the glyph
     will fit, the location where it will be placed is populated in
     outLocation.
     */
    virtual bool glyphWillFit(FT_Bitmap &bitmap, VROAtlasLocation *outLocation) = 0;
    
    /*
     Write the given Freetype glyph (with its associated bitmap) onto this atlas, at
     the provided location.
     */
    virtual void write(FT_GlyphSlot &glyph, FT_Bitmap &bitmap, const VROAtlasLocation &location,
                       std::shared_ptr<VRODriver> driver) = 0;
    
    /*
     Refresh the underlying pixel data for the texture. This should be invoked
     after writing glyphs to the atlas.
     */
    virtual void refreshTexture(std::shared_ptr<VRODriver> driver) = 0;
    
    /*
     Get the texture underlying this atlas.
     */
    std::shared_ptr<VROTexture> getTexture() const {
        return _texture;
    }
    
    /*
     Get the width and height of the texture.
     */
    virtual int getSize() const = 0;
    
protected:
    
    /*
     Texture on which the glyphs are rendered.
     */
    std::shared_ptr<VROTexture> _texture;
    
    /*
     The min and max U and V values of the occupied portions of the
     texture. We place glyphs from the top left to the bottom right,
     in reading order. _occupiedTopV represents the top of the
     lowest occupied row, and occupiedBottomV represents the bottom
     (note that occupiedBottomV > occupiedTopV since Y is inverted).
     _occupiedU represents the *right* edge of the rightmost occupied
     glyph in said row.
     
     These values range from 0 to tex size (left to right, top to
     bottom).
     */
    float _occupiedU, _occupiedBottomV, _occupiedTopV;
    
};

#endif /* VROGlyphAtlas_h */
