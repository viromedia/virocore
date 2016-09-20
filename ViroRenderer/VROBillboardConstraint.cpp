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
#include "VROCamera.h"
#include "VRONode.h"

VROMatrix4f VROBillboardConstraint::getTransform(const VRONode &node,
                                                 const VRORenderContext &context,
                                                 VROMatrix4f transform) {
    
    const VROCamera &camera = context.getCamera();
    
    VROVector3f objToCamProj = camera.getPosition().subtract(node.getTransformedPosition());
    objToCamProj.y = 0;
    objToCamProj = objToCamProj.normalize();
    
    // The direction the object is assumed to be facing (positive Z)
    VROVector3f lookAt(0, 0, 1);
    
    // Derives the axis and angle of billboard rotation
    VROVector3f upAux = lookAt.cross(objToCamProj).normalize();
    float angleCosine = lookAt.dot(objToCamProj);
    
    VROQuaternion quaternion;
    if ((angleCosine < 0.99999999) && (angleCosine > -0.99999999)) {
        quaternion = VROQuaternion::fromAngleAxis(acos(angleCosine), upAux);
    }
    else {
        quaternion = VROQuaternion::fromAngleAxis(M_PI, upAux);
    }
    
    return quaternion.getMatrix().multiply(transform);

}
