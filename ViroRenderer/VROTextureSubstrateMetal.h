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
    
    VROTextureSubstrateMetal(UIImage *image,
                             const VRORenderContextMetal &context);
    virtual ~VROTextureSubstrateMetal();
    
    id <MTLTexture> getTexture() const {
        return _texture;
    }
    
private:
  
    id <MTLTexture> _texture;
    
};

#endif /* VROTextureSubstrateMetal_h */
