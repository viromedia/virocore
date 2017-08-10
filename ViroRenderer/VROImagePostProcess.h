//
//  VROImagePostProcess.h
//  ViroKit
//
//  Created by Raj Advani on 8/10/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROImagePostProcess_h
#define VROImagePostProcess_h

#include <memory>

class VRORenderTarget;
class VRODriver;
class VROTexture;

class VROImagePostProcess {
public:
    
    VROImagePostProcess() {}
    virtual ~VROImagePostProcess() {}
    
    /*
     Bind the given texture to the given unit. It is assumed the shader used with
     this post-process has a sampler set for the unit. Returns false is the texture
     has no substrate and could not be bound.
     */
    virtual bool bindTexture(int unit, const std::shared_ptr<VROTexture> &texture,
                             std::shared_ptr<VRODriver> &driver) const = 0;
    
    /*
     Blit the given source render target to the destination render target, using the
     the post process shader. This assumes the source render target is a render-to-texture
     target.
     */
    virtual void blit(std::shared_ptr<VRORenderTarget> source,
                      std::shared_ptr<VRORenderTarget> destination,
                      std::shared_ptr<VRODriver> &driver) const = 0;
    
    /*
     Accumulate the contents of the given source render target onto the given destination
     render target, additively.
     */
    virtual void accumulate(std::shared_ptr<VRORenderTarget> source,
                            std::shared_ptr<VRORenderTarget> destination,
                            std::shared_ptr<VRODriver> &driver) const = 0;
    
    
};

#endif /* VROImagePostProcess_h */
