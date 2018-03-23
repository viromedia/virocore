//
//  VROGlyph.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROGlyph_h
#define VROGlyph_h

#include <memory>
#include "VROVector3f.h"
#include "VROAllocationTracker.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class VRODriver;
class VROTexture;

class VROGlyph {
    
public:
    
    VROGlyph() {
        ALLOCATION_TRACKER_ADD(Glyphs, 1);
    }
    virtual ~VROGlyph() {
        ALLOCATION_TRACKER_SUB(Glyphs, 1);
    }
    
    /*
     If forRendering is false, then the texture and all bitmap fields will
     not be loaded, and only getAdvance() will be available.
     
     If the variant selector is 0, then we assume this is not a variation
     sequence.
     */
    virtual bool load(FT_Face face, uint32_t charCode, uint32_t variantSelector,
                      bool forRendering, std::shared_ptr<VRODriver> driver) = 0;
    
    std::shared_ptr<VROTexture> getTexture() const {
        return _texture;
    }
    VROVector3f getSize() const {
        return _size;
    }
    VROVector3f getBearing() const {
        return _bearing;
    }
    long getAdvance() const {
        return _advance;
    }
    
    float getMinU() const {
        return _minU;
    }
    float getMaxU() const {
        return _maxU;
    }
    float getMinV() const {
        return _minV;
    }
    float getMaxV() const {
        return _maxV;
    }
    
protected:
    
    /*
     Texture on which the glyph is rendered.
     */
    std::shared_ptr<VROTexture> _texture;
    
    /*
     Size of the glyph.
     */
    VROVector3f _size;
    
    /*
     Offset from the baseline to the top-left of the glyph.
     */
    VROVector3f _bearing;
    
    /*
     Offset to advance to the next glyph.
     */
    long _advance;
    
    /*
     The min and max U and V values for the texture. These
     indicate what section of the texture corresponds to 
     this character.
     */
    float _minU, _minV;
    float _maxU, _maxV;
    
};

#endif /* VROGlyph_h */
