//
//  VROGlyphOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROGlyphOpenGL.h"
#include "VROLog.h"
#include "VROTexture.h"
#include "VROTextureSubstrateOpenGL.h"
#include "VROMath.h"

VROGlyphOpenGL::VROGlyphOpenGL() {
    
}

VROGlyphOpenGL::~VROGlyphOpenGL() {
    
}

bool VROGlyphOpenGL::load(FT_Face face, FT_ULong charCode, bool forRendering) {
    /*
     Load the glyph from freetype.
     */
    if (FT_Load_Char(face, charCode, FT_LOAD_DEFAULT)) {
        pinfo("Failed to load glyph %lu", charCode);
        return false;
    }
 
    FT_GlyphSlot &glyph = face->glyph;
    
    /*
     Each advance unit is 1/64 of a pixel so divide by 64 (>> 6) to get advance in pixels.
     */
    _advance = glyph->advance.x >> 6;
    
    if (forRendering) {
        loadTexture(face, glyph);
    }
    return true;
}

void VROGlyphOpenGL::loadTexture(FT_Face face, FT_GlyphSlot &glyph) {
    FT_Render_Glyph(glyph, FT_RENDER_MODE_LIGHT);
    FT_Bitmap &bitmap = glyph->bitmap;

    int texWidth  = VROMathRoundUpToNextPow2(bitmap.width);
    int texHeight = VROMathRoundUpToNextPow2(bitmap.rows);
    
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
    GLubyte *luminanceAlphaBitmap = (GLubyte *)malloc( sizeof(GLubyte) * 2 * texWidth * texHeight );
    for (int j = 0; j < texHeight; j++) {
        for (int i = 0; i < texWidth; i++) {
            if (i >= bitmap.width || j >= bitmap.rows) {
                luminanceAlphaBitmap[2 * (i + j * texWidth)] = 255;
                luminanceAlphaBitmap[2 * (i + j * texWidth) + 1] = 0;
            }
            else {
                luminanceAlphaBitmap[2 * (i + j * texWidth)] = 255;
                luminanceAlphaBitmap[2 * (i + j * texWidth) + 1] = bitmap.buffer[i + bitmap.width * j];
            }
        }
    }
    
    /*
     Initialize the OpenGL texture.
     */
    GLuint texture;
    GL( glGenTextures(1, &texture) );
    GL( glBindTexture(GL_TEXTURE_2D, texture) );

    GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
    GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );
    GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
    GL( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );

    GL( glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE_ALPHA, texWidth, texHeight, 0,
                     GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, luminanceAlphaBitmap) );
    
    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(
        new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture, true));
    
    _texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, std::move(substrate));
    _bearing = VROVector3f(glyph->bitmap_left, glyph->bitmap_top);
    _size = VROVector3f(bitmap.width, bitmap.rows);
    
    _minU = 0;
    _maxU = (float)bitmap.width / (float)texWidth;
    _minV = 0;
    _maxV = (float)bitmap.rows / (float)texHeight;
    
    free (luminanceAlphaBitmap);
}
