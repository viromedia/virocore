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

class VRODriver;
class VRODriverOpenGL;

class VROGlyphOpenGL : public VROGlyph {
    
public:
    
    VROGlyphOpenGL();
    virtual ~VROGlyphOpenGL();
    
    virtual bool load(FT_Face face, uint32_t charCode, uint32_t variantSelector,
                      bool forRendering, std::shared_ptr<VRODriver> driver);
    
private:
    
    void loadTexture(FT_Face face, FT_GlyphSlot &glyph,
                     std::shared_ptr<VRODriverOpenGL> driver);
    
};

#endif /* VROGlyphOpenGL_h */
