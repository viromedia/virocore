//
//  VROTexture.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/17/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROTexture_h
#define VROTexture_h

#include <UIKit/UIKit.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include <memory>

class VROTextureSubstrate;
class VRORenderContext;

class VROTexture {
    
public:
    
    /*
     Create a new VROTexture with no underlying image data.
     The image data must be injected via setSubstrate().
     */
    VROTexture();
    
    /*
     Create a new VROTexture from a UIImage.
     */
    VROTexture(UIImage *image);
    virtual ~VROTexture();
    
    VROTextureSubstrate *const getSubstrate(const VRORenderContext &context);
    void setSubstrate(std::unique_ptr<VROTextureSubstrate> substrate);
    
private:
    
    /*
     The image is retained until the texture is hydrated, after which the
     substrate is populated.
     */
    UIImage *_image;
    std::unique_ptr<VROTextureSubstrate> _substrate;
    
    void hydrate(const VRORenderContext &context);
    
};

#endif /* VROTexture_h */
