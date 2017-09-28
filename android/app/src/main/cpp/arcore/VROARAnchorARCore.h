//
//  VROARAnchorARCore.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARAnchorARCore_h
#define VROARAnchorARCore_h

#include "VROARAnchor.h"

class VROARAnchorARCore : public VROARAnchor {
public:
    
    VROARAnchorARCore(ARAnchor *anchor);
    virtual ~VROARAnchorARCore();
    
    VROMatrix4f getTransform() const;
    
private:
    
    ARAnchor *_anchor;
    
};

#endif /* VROARAnchorARCore_h */
