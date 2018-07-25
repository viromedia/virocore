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
    
    bool loadMetrics(FT_Face face, uint32_t charCode, uint32_t variantSelector);
    bool loadBitmap(FT_Face face, uint32_t charCode, uint32_t variantSelector,
                    std::vector<std::shared_ptr<VROGlyphAtlas>> *atlases,
                    std::shared_ptr<VRODriver> driver);
    bool loadOutlineBitmap(FT_Library library, FT_Face face, uint32_t charCode, uint32_t variantSelector,
                           uint32_t outlineWidth,
                           std::vector<std::shared_ptr<VROGlyphAtlas>> *atlases,
                           std::shared_ptr<VRODriver> driver);
    bool loadVector(FT_Face face, uint32_t charCode, uint32_t variantSelector);
        
private:
    
    bool loadGlyph(FT_Face face, uint32_t charCode, uint32_t variantSelector);
    
    std::vector<p2t::Point *> triangulateContour(VROVectorizer &vectorizer, int c);
    
};

#endif /* VROGlyphOpenGL_h */
