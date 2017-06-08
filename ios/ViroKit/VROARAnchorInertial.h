//
//  VROARAnchorInertial.h
//  ViroKit
//
//  Created by Raj Advani on 6/6/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#ifndef VROARAnchorInertial_h
#define VROARAnchorInertial_h

#include "VROARAnchor.h"

class VROARAnchorInertial : public VROARAnchor {
public:
    
    VROARAnchorInertial();
    virtual ~VROARAnchorInertial();
    
    VROMatrix4f getTransform() const;
    
private:
    
};

#endif /* VROARAnchorInertial_h */
