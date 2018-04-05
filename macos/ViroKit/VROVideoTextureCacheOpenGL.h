//
//  VROVideoTextureCacheOpenGL.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROVideoTextureCacheOpenGL_h
#define VROVideoTextureCacheOpenGL_h

#include "VROVideoTextureCache.h"

class VRODriverOpenGL;

static const int kVideoTextureCacheOpenGLNumTextures = 3;

class VROVideoTextureCacheOpenGL : public VROVideoTextureCache {
    
public:
    
    VROVideoTextureCacheOpenGL(CGLContextObj glContext, CGLPixelFormatObj pixelFormat,
                               std::shared_ptr<VRODriverOpenGL> driver);
    virtual ~VROVideoTextureCacheOpenGL();
    
    std::unique_ptr<VROTextureSubstrate> createTextureSubstrate(CMSampleBufferRef sampleBuffer, bool sRGB);
    std::unique_ptr<VROTextureSubstrate> createTextureSubstrate(CVPixelBufferRef pixelBuffer, bool sRGB);
    std::vector<std::unique_ptr<VROTextureSubstrate>> createYCbCrTextureSubstrates(CVPixelBufferRef pixelBuffer);
    
private:
    
    CVOpenGLTextureCacheRef _cache;
    CVOpenGLTextureRef _textureRef[kVideoTextureCacheOpenGLNumTextures];
    
    int _currentTextureIndex;
    std::shared_ptr<VRODriverOpenGL> _driver;
    
};

#endif /* VROVideoTextureCacheOpenGL_h */
