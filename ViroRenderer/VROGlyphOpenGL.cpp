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

VROGlyphOpenGL::VROGlyphOpenGL() {
    
}

VROGlyphOpenGL::~VROGlyphOpenGL() {
    
}

bool VROGlyphOpenGL::load(FT_Face face, FT_ULong charCode) {
    /*
     Disable byte alignment restriction.
     */
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    /*
     Load the glyph from freetype. Specifying FT_LOAD_RENDER makes
     freetype create the 8-bit greyscale glyph in face->glyph->bitmap.
     */
    if (FT_Load_Char(face, charCode, FT_LOAD_RENDER)) {
        pinfo("Failed to load glyph %lu", charCode);
        return false;
    }
    
    FT_Bitmap &bitmap = face->glyph->bitmap;
    
    /*
     Initialize the OpenGL texture. The glyphs are 8-bit so use GL_RED
     for the format.
     */
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmap.width, bitmap.rows,
                 0, GL_RED, GL_UNSIGNED_BYTE, bitmap.buffer);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    std::unique_ptr<VROTextureSubstrate> substrate = std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture, true));
    
    _texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, std::move(substrate));
    _size = VROVector3f(bitmap.width, bitmap.rows);
    _bearing = VROVector3f(face->glyph->bitmap_left, face->glyph->bitmap_top);
    _advance = face->glyph->advance.x;
    
    return true;
}
