//
//  VROGlyphAtlas.h
//  ViroRenderer
//
//  Created by Raj Advani on 7/10/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
    virtual void write(FT_Bitmap &bitmap, const VROAtlasLocation &location, std::shared_ptr<VRODriver> driver) = 0;
    
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
     Return true if this atlas is for outline glyphs.
     */
    bool isOutline() const { return _outline; }
    
    /*
     Get the width and height of the texture.
     */
    virtual int getSize() const = 0;
    
protected:
    
    /*
     True if this atlas is used by outline glyphs (this should impact
     render order).
     */
    bool _outline;
    
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
