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

VROVideoTextureCacheOpenGL::VROVideoTextureCacheOpenGL(CVEAGLContext eaglContext,
                                                       std::shared_ptr<VRODriverOpenGL> driver)
    : _currentTextureIndex(0),
      _driver(driver) {
    CVReturn textureCacheError = CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, NULL, eaglContext,
                                                              NULL, &_cache);
    if (textureCacheError) {
        pinfo("ERROR: Couldnt create a texture cache");
        pabort();
    }
    for (int i = 0; i < kVideoTextureCacheOpenGLNumTextures; i++) {
        _textureRef[i] = NULL;
    }
    ALLOCATION_TRACKER_ADD(VideoTextureCaches, 1);
}

VROVideoTextureCacheOpenGL::~VROVideoTextureCacheOpenGL() {
    for (int i = 0; i < kVideoTextureCacheOpenGLNumTextures; i++) {
        if (_textureRef[i] != NULL) {
            CVBufferRelease(_textureRef[i]);
        }
    }
    CFRelease(_cache);
    ALLOCATION_TRACKER_SUB(VideoTextureCaches, 1);
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheOpenGL::createTextureSubstrate(CMSampleBufferRef sampleBuffer) {
    CVBufferRelease(_textureRef[_currentTextureIndex]);
    _textureRef[_currentTextureIndex] = NULL;
    
    _currentTextureIndex = (_currentTextureIndex + 1) % kVideoTextureCacheOpenGLNumTextures;
    CVOpenGLESTextureCacheFlush(_cache, 0);

    CVImageBufferRef sourceImageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    int width  = (int) CVPixelBufferGetWidth(sourceImageBuffer);
    int height = (int) CVPixelBufferGetHeight(sourceImageBuffer);
    
    CVReturn error;
    error = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, sourceImageBuffer,
                                                         NULL, GL_TEXTURE_2D, GL_RGBA, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0,
                                                         &_textureRef[_currentTextureIndex]);
    if (error) {
        pabort("Failed to create texture from image");
    }
    
    GLuint texture = CVOpenGLESTextureGetName(_textureRef[_currentTextureIndex]);
    if (texture == 0) {
        pabort("Failed to retreive texture from texture ref");
    }
    
    return std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture, _driver, false));
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheOpenGL::createTextureSubstrate(CVPixelBufferRef pixelBuffer) {
    CVBufferRelease(_textureRef[_currentTextureIndex]);
    _textureRef[_currentTextureIndex] = NULL;
    
    _currentTextureIndex = (_currentTextureIndex + 1) % kVideoTextureCacheOpenGLNumTextures;
    CVOpenGLESTextureCacheFlush(_cache, 0);
    
    int width  = (int) CVPixelBufferGetWidth(pixelBuffer);
    int height = (int) CVPixelBufferGetHeight(pixelBuffer);
    
    CVReturn error;
    error = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, pixelBuffer,
                                                         NULL, GL_TEXTURE_2D, GL_RGBA, width, height, GL_BGRA, GL_UNSIGNED_BYTE, 0,
                                                         &_textureRef[_currentTextureIndex]);
    if (error) {
        pabort("Failed to create texture from image");
    }
    GLuint texture = CVOpenGLESTextureGetName(_textureRef[_currentTextureIndex]);
    if (texture == 0) {
        pabort("Failed to retreive texture from texture ref");
    }
    
    return std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture, _driver, false));
}

std::vector<std::unique_ptr<VROTextureSubstrate>> VROVideoTextureCacheOpenGL::createYCbCrTextureSubstrates(CVPixelBufferRef pixelBuffer) {
    std::unique_ptr<VROTextureSubstrate> textureY = createTextureSubstrate(pixelBuffer, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
    std::unique_ptr<VROTextureSubstrate> textureCbCr = createTextureSubstrate(pixelBuffer, GL_LUMINANCE_ALPHA, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, 1);
    
    std::vector<std::unique_ptr<VROTextureSubstrate>> substrates;
    substrates.push_back(std::move(textureY));
    substrates.push_back(std::move(textureCbCr));
    
    return substrates;
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheOpenGL::createTextureSubstrate(CVPixelBufferRef pixelBuffer,
                                                                                        int internalFormat, GLenum format, GLenum type,
                                                                                        int planeIndex) {
    CVBufferRelease(_textureRef[_currentTextureIndex]);
    _textureRef[_currentTextureIndex] = NULL;
    
    _currentTextureIndex = (_currentTextureIndex + 1) % kVideoTextureCacheOpenGLNumTextures;
    CVOpenGLESTextureCacheFlush(_cache, 0);
    
    int width  = (int) CVPixelBufferGetWidthOfPlane(pixelBuffer, planeIndex);
    int height = (int) CVPixelBufferGetHeightOfPlane(pixelBuffer, planeIndex);
    
    CVReturn error;
    error = CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, pixelBuffer,
                                                         NULL, GL_TEXTURE_2D, internalFormat, width, height, format, type, planeIndex,
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
    
    return std::unique_ptr<VROTextureSubstrateOpenGL>(new VROTextureSubstrateOpenGL(GL_TEXTURE_2D, texture, _driver, false));
}
