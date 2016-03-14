//
//  VROBillboardConstraint.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/9/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROBillboardConstraint.h"
#include "VROMatrix4f.h"
#include "VROVector3f.h"
#include "VROQuaternion.h"
#include "VRONode.h"

VROMatrix4f VROBillboardConstraint::getTransform(const VRONode &node, VROMatrix4f transform,
                                                 const VRORenderContext &context) {
    
    VROVector3f up = { 0, 1, 0 };
    VROVector3f look = node.getPosition().normalize(); // TODO if camera moves, need to change this
    VROVector3f right = up.cross(look);
    
    VROMatrix4f matrix;
    matrix[0] = -right.x;
    matrix[1] = -right.y;
    matrix[2] = -right.z;
    matrix[3] = 0;
    matrix[4] = up.x;
    matrix[5] = up.y;
    matrix[6] = up.z;
    matrix[7] = 0;
    matrix[8] = look.x;
    matrix[9] = look.y;
    matrix[10] = look.z;
    matrix[11] = 0;
    matrix[12] = 0;
    matrix[13] = 0;
    matrix[14] = 0;
    matrix[15] = 1;
 
    return transform.multiply(matrix);
}
