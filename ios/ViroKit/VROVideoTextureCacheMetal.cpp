//
//  VROVideoTextureCacheMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 5/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

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

#endif
