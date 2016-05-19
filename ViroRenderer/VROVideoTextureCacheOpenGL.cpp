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

VROVideoTextureCacheOpenGL::VROVideoTextureCacheOpenGL(CVEAGLContext eaglContext) {
    CVReturn textureCacheError = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, eaglContext,
                                                              NULL, &_cache);
    if (textureCacheError) {
        pinfo("ERROR: Couldnt create a texture cache");
        pabort();
    }
}

VROVideoTextureCacheOpenGL::~VROVideoTextureCacheOpenGL() {
    
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheOpenGL::createTextureSubstrate(CMSampleBufferRef sampleBuffer) {
    CVReturn error;
    
    CVImageBufferRef sourceImageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    int width  = (int) CVPixelBufferGetWidth(sourceImageBuffer);
    int height = (int) CVPixelBufferGetHeight(sourceImageBuffer);
    
    CVOpenGLESTextureRef textureRef;
    error = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, sourceImageBuffer,
                                                         NULL, GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0, &textureRef);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    GLuint texture = CVOpenGLESTextureGetName(textureRef);
    if (texture == 0) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    CVBufferRelease(textureRef);
    return std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture));
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheOpenGL::createTextureSubstrate(CVPixelBufferRef pixelBuffer) {
    CVReturn error;
    
    int width  = (int) CVPixelBufferGetWidth(pixelBuffer);
    int height = (int) CVPixelBufferGetHeight(pixelBuffer);
    
    CVOpenGLESTextureRef textureRef;
    error = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, pixelBuffer,
                                                         NULL, GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0, &textureRef);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    GLuint texture = CVOpenGLESTextureGetName(textureRef);
    if (texture == 0) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    CVBufferRelease(textureRef);
    return std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture));
}