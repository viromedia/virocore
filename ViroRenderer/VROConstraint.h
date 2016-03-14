//
//  VROConstraint.h
//  ViroRenderer
//
//  Created by Raj Advani on 3/9/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROConstraint_h
#define VROConstraint_h

#include <stdio.h>
#include <memory>

class VRONode;
class VROMatrix4f;
class VROQuaternion;
class VROVector3f;
class VRORenderContext;

class VROConstraint {
    
public:
    
    virtual VROMatrix4f getTransform(const VRONode &node, VROMatrix4f transform,
                                     const VRORenderContext &context) = 0;
    
};

#endif /* VROConstraint_h */
