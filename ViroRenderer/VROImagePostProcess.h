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
#include <vector>

class VRORenderTarget;
class VRODriver;
class VROTexture;

class VROImagePostProcess {
public:
    
    VROImagePostProcess() {}
    virtual ~VROImagePostProcess() {}
    
    /*
     Set to true to flip the result image vertically.
     */
    virtual void setVerticalFlip(bool flip) = 0;
    
    /*
     Bind the given textures and blit to the destination render target, using the
     post-process shader.
     
     The provided textures will be bound to samplers (texture units) 0 to N.
     */
    virtual void blit(std::vector<std::shared_ptr<VROTexture>> textures,
                      std::shared_ptr<VRORenderTarget> destination,
                      std::shared_ptr<VRODriver> &driver) = 0;
    
    /*
     Prepare for a post-process that will use the same shader multiple times
     on different FBOs. This minimizes state changes by performing the
     configuration (binding the shader and vertex arrays) once for multiple
     blit operations.
     
     Used in conjunction with blitOpt() and end().
     */
    virtual void begin(std::shared_ptr<VRODriver> &driver) = 0;
    virtual void blitOpt(std::vector<std::shared_ptr<VROTexture>> textures,
                         std::shared_ptr<VRORenderTarget> destination,
                         std::shared_ptr<VRODriver> &driver) = 0;
    virtual void end(std::shared_ptr<VRODriver> &driver) = 0;
    
};

#endif /* VROImagePostProcess_h */
