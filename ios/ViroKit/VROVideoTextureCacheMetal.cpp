//
//  VROVideoTextureCacheMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

#include "VROVideoTextureCacheMetal.h"
#if VRO_METAL

#include "VROLog.h"
#include "VROTextureSubstrateMetal.h"

VROVideoTextureCacheMetal::VROVideoTextureCacheMetal(id <MTLDevice> device) {
    CVReturn textureCacheError = CVMetalTextureCacheCreate(kCFAllocatorDefault, NULL, device,
                                                           NULL, &_cache);
    if (textureCacheError) {
        pinfo("ERROR: Couldnt create a texture cache");
        pabort();
    }
}

VROVideoTextureCacheMetal::~VROVideoTextureCacheMetal() {
    
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheMetal::createTextureSubstrate(CMSampleBufferRef sampleBuffer) {
    CVReturn error;
    
    CVImageBufferRef sourceImageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);
    size_t width = CVPixelBufferGetWidth(sourceImageBuffer);
    size_t height = CVPixelBufferGetHeight(sourceImageBuffer);
    
    CVMetalTextureRef textureRef;
    error = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, sourceImageBuffer,
                                                      NULL, MTLPixelFormatBGRA8Unorm, width, height, 0, &textureRef);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    id <MTLTexture> texture = CVMetalTextureGetTexture(textureRef);
    if (!texture) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    CVBufferRelease(textureRef);
    return std::unique_ptr<VROTextureSubstrateMetal>(new VROTextureSubstrateMetal(texture));
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheMetal::createTextureSubstrate(CVPixelBufferRef pixelBuffer) {
    CVReturn error;
    
    size_t width = CVPixelBufferGetWidth(pixelBuffer);
    size_t height = CVPixelBufferGetHeight(pixelBuffer);
    
    CVMetalTextureRef textureRef;
    error = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, pixelBuffer,
                                                      NULL, MTLPixelFormatBGRA8Unorm, width, height, 0, &textureRef);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    id <MTLTexture> videoTexture = CVMetalTextureGetTexture(textureRef);
    if (!videoTexture) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    CVBufferRelease(textureRef);
    return std::unique_ptr<VROTextureSubstrateMetal>(new VROTextureSubstrateMetal(videoTexture));
}

std::vector<std::unique_ptr<VROTextureSubstrate>> VROVideoTextureCacheMetal::createYCbCrTextureSubstrates(CVPixelBufferRef pixelBuffer) {
    std::unique_ptr<VROTextureSubstrate> textureY = createTextureSubstrate(pixelBuffer, MTLPixelFormatR8Unorm, 0);
    std::unique_ptr<VROTextureSubstrate> textureCbCr = createTextureSubstrate(pixelBuffer, MTLPixelFormatRG8Unorm, 1);
    
    std::vector<std::unique_ptr<VROTextureSubstrate>> substrates;
    substrates.push_back(std::move(textureY));
    substrates.push_back(std::move(textureCbCr));
    
    return substrates;
}

std::unique_ptr<VROTextureSubstrate> VROVideoTextureCacheMetal::createTextureSubstrate(CVPixelBufferRef pixelBuffer, MTLPixelFormat pixelFormat,
                                                                                       int planeIndex) {
    CVReturn error;
    
    size_t width = CVPixelBufferGetWidthOfPlane(pixelBuffer, planeIndex);
    size_t height = CVPixelBufferGetHeightOfPlane(pixelBuffer, planeIndex);
    
    CVMetalTextureRef textureRef;
    error = CVMetalTextureCacheCreateTextureFromImage(kCFAllocatorDefault, _cache, pixelBuffer,
                                                      NULL, pixelFormat, width, height, planeIndex, &textureRef);
    
    if (error) {
        pinfo("ERROR: Couldnt create texture from image");
        pabort();
    }
    
    id <MTLTexture> videoTexture = CVMetalTextureGetTexture(textureRef);
    if (!videoTexture) {
        pinfo("ERROR: Couldn't get texture from texture ref");
        pabort();
    }
    
    CVBufferRelease(textureRef);
    return std::unique_ptr<VROTextureSubstrateMetal>(new VROTextureSubstrateMetal(videoTexture));
    
}

#endif
