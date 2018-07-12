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
                      bool forRendering, std::vector<std::shared_ptr<VROGlyphAtlas>> *atlases,
                      std::shared_ptr<VRODriver> driver);
    
    std::shared_ptr<VROTexture> getTexture() const;
    
private:
    
};

#endif /* VROGlyphOpenGL_h */
