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

class VROVideoTextureCacheOpenGL : public VROVideoTextureCache {
    
public:
    
    VROVideoTextureCacheOpenGL(CVEAGLContext eaglContext);
    virtual ~VROVideoTextureCacheOpenGL();
    
    std::unique_ptr<VROTextureSubstrate> createTextureSubstrate(CMSampleBufferRef sampleBuffer);
    std::unique_ptr<VROTextureSubstrate> createTextureSubstrate(CVPixelBufferRef pixelBuffer);
    
private:
    
    CVOpenGLESTextureCacheRef _cache;
    
};

#endif /* VROVideoTextureCacheOpenGL_h */
