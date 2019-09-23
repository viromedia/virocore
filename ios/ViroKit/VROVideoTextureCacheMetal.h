//
//  VROVideoTextureCacheMetal.h
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
