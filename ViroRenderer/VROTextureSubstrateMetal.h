//
//  VROTextureSubstrateMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 12/4/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROTextureSubstrateMetal_h
#define VROTextureSubstrateMetal_h

#include <UIKit/UIKit.h>
#include <Metal/Metal.h>
#include "VROTextureSubstrate.h"

class VRORenderContextMetal;

class VROTextureSubstrateMetal : public VROTextureSubstrate {
    
public:
    
    /*
     Create a new texture substrate with the given underlying MTLTexture.
     */
    VROTextureSubstrateMetal(id <MTLTexture> texture) :
        _texture(texture)
    {}
    
    /*
     Create a new Metal texture from the given UIImage.
     */
    VROTextureSubstrateMetal(UIImage *image, const VRORenderContextMetal &context);
    virtual ~VROTextureSubstrateMetal();
    
    id <MTLTexture> getTexture() const {
        return _texture;
    }
    void setTexture(id <MTLTexture> texture) {
        _texture = texture;
    }
    
private:
  
    id <MTLTexture> _texture;
    
};

#endif /* VROTextureSubstrateMetal_h */
