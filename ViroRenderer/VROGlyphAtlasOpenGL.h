//
//  VROGlyphAtlasOpenGL.h
//  ViroKit
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

#ifndef VROGlyphAtlasOpenGL_h
#define VROGlyphAtlasOpenGL_h

#include "VROGlyphAtlas.h"
#include "VROOpenGL.h"

class VRODriver;
class VRODriverOpenGL;

class VROGlyphAtlasOpenGL : public VROGlyphAtlas {
    
public:
    
    VROGlyphAtlasOpenGL(bool isOutline);
    virtual ~VROGlyphAtlasOpenGL();
    
    void refreshTexture(std::shared_ptr<VRODriver> driver);
    bool glyphWillFit(FT_Bitmap &bitmap, VROAtlasLocation *outLocation);
    void write(FT_Bitmap &bitmap, const VROAtlasLocation &location, std::shared_ptr<VRODriver> driver);
    int getSize() const;
    
private:
    
    GLuint _textureId;
    GLubyte *_luminanceAlphaBitmap;
    
    void loadTexture(FT_Face face, FT_GlyphSlot &glyph,
                     std::shared_ptr<VRODriverOpenGL> driver);
    
};

#endif /* VROGlyphAtlasOpenGL_h */
