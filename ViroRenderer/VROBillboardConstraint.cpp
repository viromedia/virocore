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
    
    VROVector3f objToCamProj = node.getTransformedPosition().subtract(camera.getPosition());
    objToCamProj.y = 0;
    objToCamProj = objToCamProj.normalize();
    
    VROVector3f lookAt(0, 0, -1);
    VROVector3f upAux = lookAt.cross(objToCamProj).normalize();
    float angleCosine = lookAt.dot(objToCamProj);
        
    if ((angleCosine < 0.9999) && (angleCosine > -0.9999)) {        
        VROQuaternion quaternion = VROQuaternion::fromAngleAxis(acos(angleCosine), upAux);
        return transform.multiply(quaternion.getMatrix());
    }
    else {
        return transform;
    }
}
