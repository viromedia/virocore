//
//  VROGlyphAtlasOpenGL.h
//  ViroKit
//
//  Created by Raj Advani on 7/10/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#ifndef VROGlyphAtlasOpenGL_h
#define VROGlyphAtlasOpenGL_h

#include "VROGlyphAtlas.h"
#include "VROOpenGL.h"

class VRODriver;
class VRODriverOpenGL;

class VROGlyphAtlasOpenGL : public VROGlyphAtlas {
    
public:
    
    VROGlyphAtlasOpenGL();
    virtual ~VROGlyphAtlasOpenGL();
    
    void refreshTexture(std::shared_ptr<VRODriver> driver);
    bool glyphWillFit(FT_Bitmap &bitmap, VROAtlasLocation *outLocation);
    void write(FT_GlyphSlot &glyph, FT_Bitmap &bitmap,
               const VROAtlasLocation &location, std::shared_ptr<VRODriver> driver);
    int getSize() const;
    
private:
    
    GLuint _textureId;
    GLubyte *_luminanceAlphaBitmap;
    
    void loadTexture(FT_Face face, FT_GlyphSlot &glyph,
                     std::shared_ptr<VRODriverOpenGL> driver);
    
};

#endif /* VROGlyphAtlasOpenGL_h */
