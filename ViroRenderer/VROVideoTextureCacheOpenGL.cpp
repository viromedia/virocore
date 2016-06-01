//
//  VROVideoTextureCacheOpenGL.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROVideoTextureCacheOpenGL.h"
#include "VROLog.h"
#include "VROTextureSubstrateOpenGL.h"

VROVideoTextureCacheOpenGL::VROVideoTextureCacheOpenGL(CVEAGLContext eaglContext)
    : _currentTextureIndex(0) {
    CVReturn textureCacheError = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, eaglContext,
                                                              NULL, &_cache);
    if (textureCacheError) {
        pinfo("ERROR: Couldnt create a texture cache");
        pabort();
    }
        
    for (int i = 0; i < kVideoTextureCacheOpenGLNumTextures; i++) {
        _textureRef[i] = NULL;
    }
}

VROVideoTextureCacheOpenGL::~VROVideoTextureCacheOpenGL() {
    for (int i = 0; i < kVideoTextureCacheOpenGLNumTextures; i++) {
        CVBufferRelease(_textureRef[i]);
    }
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheOpenGL::createTextureSubstrate(CMSampleBufferRef sampleBuffer) {
    CVBufferRelease(_textureRef[_currentTextureIndex]);
    _currentTextureIndex = (_currentTextureIndex + 1) % kVideoTextureCacheOpenGLNumTextures;
    CVOpenGLESTextureCacheFlush(_cache, 0);

    CVImageBufferRef sourceImageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    int width  = (int) CVPixelBufferGetWidth(sourceImageBuffer);
    int height = (int) CVPixelBufferGetHeight(sourceImageBuffer);
    
    CVReturn error;
    error = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, sourceImageBuffer,
                                                         NULL, GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0,
                                                         &_textureRef[_currentTextureIndex]);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    GLuint texture = CVOpenGLESTextureGetName(_textureRef[_currentTextureIndex]);
    if (texture == 0) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    return std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture, false));
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheOpenGL::createTextureSubstrate(CVPixelBufferRef pixelBuffer) {
    CVBufferRelease(_textureRef[_currentTextureIndex]);
    _currentTextureIndex = (_currentTextureIndex + 1) % kVideoTextureCacheOpenGLNumTextures;
    CVOpenGLESTextureCacheFlush(_cache, 0);
    
    int width  = (int) CVPixelBufferGetWidth(pixelBuffer);
    int height = (int) CVPixelBufferGetHeight(pixelBuffer);
    
    CVReturn error;
    error = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, pixelBuffer,
                                                         NULL, GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0,
                                                         &_textureRef[_currentTextureIndex]);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    GLuint texture = CVOpenGLESTextureGetName(_textureRef[_currentTextureIndex]);
    if (texture == 0) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    return std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture, false));
}