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
#include <vector>
#include "VROVector3f.h"
#include "VROAllocationTracker.h"
#include <ft2build.h>
#include FT_FREETYPE_H

class VRODriver;
class VROTexture;
class VROGlyphAtlas;
class VROAtlasLocation;

class VROGlyph {
    
public:
    
    VROGlyph() {
        ALLOCATION_TRACKER_ADD(Glyphs, 1);
    }
    virtual ~VROGlyph() {
        ALLOCATION_TRACKER_SUB(Glyphs, 1);
    }
    
    /*
     Load the glyph identified by the given FT_Face. If forRendering is false,
     then only getAdvance() will be available after this operation. If
     forRendering is true, then the glyph will additionally be written to the
     last VROGlyphAtlas provided in the given vector. If the glyph does not
     fit in said vector, then a new VROGlyphAtlas will be created and pushed
     to the back of the vector.
     
     If the variant selector is 0, then we assume this is not a variation
     sequence.
     */
    virtual bool load(FT_Face face, uint32_t charCode, uint32_t variantSelector,
                      bool forRendering, std::vector<std::shared_ptr<VROGlyphAtlas>> *atlases,
                      std::shared_ptr<VRODriver> driver) = 0;
    
    virtual std::shared_ptr<VROTexture> getTexture() const = 0;
    const std::shared_ptr<VROGlyphAtlas> getAtlas() const {
        return _atlas;
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
     Atlas on which the glyph is rendered.
     */
    std::shared_ptr<VROGlyphAtlas> _atlas;
    
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
