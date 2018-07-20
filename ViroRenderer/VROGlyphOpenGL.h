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
class VROVectorizer;

namespace p2t {
    class Point;
}

class VROGlyphOpenGL : public VROGlyph {
    
public:
    
    VROGlyphOpenGL();
    virtual ~VROGlyphOpenGL();
    
    virtual bool load(FT_Face face, uint32_t charCode, uint32_t variantSelector,
                      VROGlyphRenderMode renderMode, std::vector<std::shared_ptr<VROGlyphAtlas>> *atlases,
                      std::shared_ptr<VRODriver> driver);
    
    std::shared_ptr<VROTexture> getTexture() const;
    
private:
    
    bool loadBitmap(FT_GlyphSlot &glyph, uint32_t charCode,
                    std::vector<std::shared_ptr<VROGlyphAtlas>> *atlases,
                    std::shared_ptr<VRODriver> driver);
    bool loadVector(FT_GlyphSlot &glyph);
    
    std::vector<p2t::Point *> triangulateContour(VROVectorizer &vectorizer, int c);
    
};

#endif /* VROGlyphOpenGL_h */
