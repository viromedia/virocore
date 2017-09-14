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
     Bind the given texture to the given unit. It is assumed the shader used with
     this post-process has a sampler set for the unit. Returns false is the texture
     has no substrate and could not be bound.
     */
    virtual bool bindTexture(int unit, const std::shared_ptr<VROTexture> &texture,
                             std::shared_ptr<VRODriver> &driver) = 0;
    
    /*
     Blit the given attachment of the source render target to the destination
     render target, using the the post process shader. This assumes the source
     render target is a render-to-texture target.
     
     The provided textures will be bound to samplers (texture units) 1 to N.
     Texture unit 0 (sampler 0) will receive the source render target's texture.
     */
    virtual void blit(std::shared_ptr<VRORenderTarget> source, int attachment,
                      std::shared_ptr<VRORenderTarget> destination,
                      std::vector<std::shared_ptr<VROTexture>> textures,
                      std::shared_ptr<VRODriver> &driver) = 0;
    
    /*
     Prepare for a post-process that will use the same shader multiple times
     on different FBOs. This minimizes state changes by performing the
     configuration (binding the shader and vertex arrays) once for multiple
     blit operations.
     
     Used in conjunction with blitOpt() and end().
     */
    virtual void begin(std::shared_ptr<VRODriver> &driver) = 0;
    virtual void blitOpt(std::shared_ptr<VRORenderTarget> source, int attachment,
                         std::shared_ptr<VRORenderTarget> destination,
                         std::vector<std::shared_ptr<VROTexture>> textures,
                         std::shared_ptr<VRODriver> &driver) = 0;
    virtual void end(std::shared_ptr<VRODriver> &driver) = 0;
    
};

#endif /* VROImagePostProcess_h */
