//
//  VROGlyphOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROGlyphOpenGL_h
#define VROGlyphOpenGL_h

#include "VROGlyph.h"
#include "VROOpenGL.h"

class VROGlyphOpenGL : public VROGlyph {
    
public:
    
    VROGlyphOpenGL();
    virtual ~VROGlyphOpenGL();
    
    virtual bool load(FT_Face face, FT_ULong charCode, bool forRendering);
    
private:
    
    void loadTexture(FT_Face face, FT_GlyphSlot &glyph);
    
};

#endif /* VROGlyphOpenGL_h */
