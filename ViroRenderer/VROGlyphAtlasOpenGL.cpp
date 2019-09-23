//
//  VROGlyphAtlasOpenGL.cpp
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

#include "VROGlyphAtlasOpenGL.h"
#include "VROLog.h"
#include "VROTexture.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROMath.h"
#include "VRODriverOpenGL.h"
#include "VRODefines.h"

static const int kGlyphAtlasTextureSize = 512;
static const int kGlyphPadding = 8;

VROGlyphAtlasOpenGL::VROGlyphAtlasOpenGL(bool isOutline) :
    _textureId(0) {
    _outline = isOutline;
    _luminanceAlphaBitmap = (GLubyte *)malloc( sizeof(GLubyte) * 2 *
                                               kGlyphAtlasTextureSize * kGlyphAtlasTextureSize );
        
    for (int j = 0; j < kGlyphAtlasTextureSize; j++) {
        for (int i = 0; i < kGlyphAtlasTextureSize; i++) {
            _luminanceAlphaBitmap[2 * (i + j * kGlyphAtlasTextureSize)] = 255;
            _luminanceAlphaBitmap[2 * (i + j * kGlyphAtlasTextureSize) + 1] = 0;
        }
    }
}

VROGlyphAtlasOpenGL::~VROGlyphAtlasOpenGL() {
    free (_luminanceAlphaBitmap);
}

int VROGlyphAtlasOpenGL::getSize() const {
    return kGlyphAtlasTextureSize;
}

bool VROGlyphAtlasOpenGL::glyphWillFit(FT_Bitmap &bitmap, VROAtlasLocation *outLocation) {
    int texWidth  = bitmap.width;
    int texHeight = bitmap.rows;
    
    int minU = 0;
    int minV = 0;
    
    int rowWidthRemaining = kGlyphAtlasTextureSize - _occupiedU;
    int columnHeightRemaining = kGlyphAtlasTextureSize - _occupiedBottomV;
    
    if (rowWidthRemaining < texWidth + kGlyphPadding) {
        // If we've reached the end of this row, see if there's room for a new row
        if (columnHeightRemaining < texHeight + kGlyphPadding) {
            // There isn't, return false: the glyph will not fit in this atlas
            return false;
        } else {
            // New row
            minU = kGlyphPadding;
            minV = _occupiedBottomV + kGlyphPadding;
            
            _occupiedU = minU + texWidth;
            _occupiedTopV = minV;
        }
    } else {
        // There is enough X room for the glyph, check Y room
        if (columnHeightRemaining < texHeight + kGlyphPadding) {
            // There isn't, return false: the glyph will not fit in this atlas
            return false;
        } else {
            // Add the glyph to the current row
            minU = _occupiedU + kGlyphPadding;
            minV = _occupiedTopV;
            
            _occupiedU = minU + texWidth;
            _occupiedBottomV = fmax(_occupiedBottomV, minV + texHeight);
        }
    }
    
    outLocation->minU = minU;
    outLocation->maxU = minU + texWidth;
    outLocation->minV = minV;
    outLocation->maxV = minV + texHeight;
    
    return true;
}

void VROGlyphAtlasOpenGL::write(FT_Bitmap &bitmap, const VROAtlasLocation &location,
                                std::shared_ptr<VRODriver> driver) {
   
    int texWidth  = bitmap.width;
    int texHeight = bitmap.rows;
    
    int minU = location.minU;
    int minV = location.minV;
    
    /*
     Created a power of 2 luminance alpha bitmap for maximum compatibility,
     padding the edges not used by the glyph with zeros.
     
     Each pixel in a luminance alpha bitmap is an 8-bit luminance and 8-bit
     alpha pair. The GPU assigns the 8-bit luminance value to the R, G, B
     channels of the texture, and the 8-bit alpha value to the A channel.
     
     We set the luminance portion to 1.0 so RGB are each 1.0. This way the color
     of the text is determined entirely by the material's diffuse color. The
     alpha value of the texture is taken from the glyph's value.
     */
    for (int j = 0; j < texHeight; j++) {
        for (int i = 0; i < texWidth; i++) {
            _luminanceAlphaBitmap[2 * (minU + i + (j + minV) * kGlyphAtlasTextureSize) + 0] = 255;
            _luminanceAlphaBitmap[2 * (minU + i + (j + minV) * kGlyphAtlasTextureSize) + 1] = bitmap.buffer[i + bitmap.width * j];
        }
    }
}

void VROGlyphAtlasOpenGL::refreshTexture(std::shared_ptr<VRODriver> driver) {
    bool isNew = false;
    if (!_textureId) {
        GL( glGenTextures(1, &_textureId) );
        isNew = true;
    }
    
    GL( glBindTexture(GL_TEXTURE_2D, _textureId) );
    
    if (isNew) {
        GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
        GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
        GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
        GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR) );
        GL( glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, kGlyphAtlasTextureSize, kGlyphAtlasTextureSize, 0,
                         GL_RG, GL_UNSIGNED_BYTE, _luminanceAlphaBitmap) );
    } else {
        GL( glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kGlyphAtlasTextureSize, kGlyphAtlasTextureSize,
                            GL_RG, GL_UNSIGNED_BYTE, _luminanceAlphaBitmap) );
    }
    GL( glGenerateMipmap(GL_TEXTURE_2D) );
    
    if (isNew) {
        std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, _textureId, std::dynamic_pointer_cast<VRODriverOpenGL>(driver), true));
        _texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, VROTextureInternalFormat::RG8,
                                                std::move(substrate));
    }
}
