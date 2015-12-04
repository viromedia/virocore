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

class VROTextureSubstrate;
class VRORenderContext;

class VROTexture {
    
public:
    
    VROTexture(UIImage *image);
    virtual ~VROTexture();
    
    void hydrate(const VRORenderContext &context);
    
    VROTextureSubstrate *const getSubstrate() const {
        return _substrate;
    }
    
private:
    
    /*
     The image is retained until the texture is hydrated, after which the
     substrate is populated.
     */
    UIImage *_image;
    VROTextureSubstrate *_substrate;
    
};

#endif /* VROTexture_h */
