//
//  VROVideoTextureCacheMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 5/19/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROVideoTextureCacheMetal_h
#define VROVideoTextureCacheMetal_h

#include "VRODefines.h"
#if VRO_METAL

#include <Metal/Metal.h>
#include "VROVideoTextureCache.h"

class VROVideoTextureCacheMetal : public VROVideoTextureCache {
    
public:
    
    VROVideoTextureCacheMetal(id <MTLDevice> device);
    virtual ~VROVideoTextureCacheMetal();
    
    std::unique_ptr<VROTextureSubstrate> createTextureSubstrate(CMSampleBufferRef sampleBuffer);
    std::unique_ptr<VROTextureSubstrate> createTextureSubstrate(CVPixelBufferRef pixelBuffer);
    std::vector<std::unique_ptr<VROTextureSubstrate>> createYCbCrTextureSubstrates(CVPixelBufferRef pixelBuffer);
    
private:
    
    CVMetalTextureCacheRef _cache;
    std::unique_ptr<VROTextureSubstrate> createTextureSubstrate(CVPixelBufferRef pixelBuffer, MTLPixelFormat pixelFormat,
                                                                int planeIndex);
    
};

#endif
#endif /* VROVideoTextureCacheMetal_h */
