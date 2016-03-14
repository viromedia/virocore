//
//  VROTransformConstraint.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/9/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROTransformConstraint.h"
#include "VROMatrix4f.h"

VROMatrix4f VROTransformConstraint::getTransform(const VRONode &node, VROMatrix4f transform,
                                                 const VRORenderContext &context) {
    
    return _function(node, transform, context);
}