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
#include <ft2build.h>
#include FT_FREETYPE_H

class VROTexture;

class VROGlyph {
    
public:
    
    VROGlyph() {}
    virtual ~VROGlyph() {}
    
    virtual bool load(FT_Face face, FT_ULong charCode) = 0;
    
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
    
};

#endif /* VROGlyph_h */
