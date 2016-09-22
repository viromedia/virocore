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
    
    if (_freeAxis == VROBillboardAxis::All) {
        // Billboard with free Y axis
        VROVector3f objToCam = camera.getPosition().subtract(node.getTransformedPosition());
        
        VROVector3f objToCamProj = objToCam;
        objToCamProj.y = 0;
        objToCamProj = objToCamProj.normalize();

        VROQuaternion quaternionY = computeAxisRotation({ 0, 1, 0 }, objToCamProj);
        
        // Billboard again, with free X axis
        objToCam = objToCam.normalize();
        float angleCosine = objToCam.dot(objToCamProj);
        
        VROVector3f axis;
        if (objToCam.y < 0) {
            axis = { 1, 0, 0 };
        }
        else {
            axis = { -1, 0, 0 };
        }
        
        VROQuaternion quaternionX;
        if (angleCosine > 0.99999999) {
            quaternionX = VROQuaternion::fromAngleAxis(0, axis);
        }
        else if (angleCosine < -0.99999999) {
            quaternionX = VROQuaternion::fromAngleAxis(M_PI, axis);
        }
        else {
            quaternionX = VROQuaternion::fromAngleAxis(acos(angleCosine), axis);
        }
        
        VROQuaternion composed = quaternionX * quaternionY;
        return composed.getMatrix().multiply(transform);
    }
    else {
        VROVector3f objToCamProj = camera.getPosition().subtract(node.getTransformedPosition());
        VROVector3f defaultAxis;

        if (_freeAxis == VROBillboardAxis::X) {
            objToCamProj.x = 0;
            defaultAxis = { 1, 0, 0 };
        }
        else if (_freeAxis == VROBillboardAxis::Y) {
            objToCamProj.y = 0;
            defaultAxis = { 0, 1, 0 };
        }
        else if (_freeAxis == VROBillboardAxis::Z) {
            objToCamProj.z = 0;
            defaultAxis = { 0, 0, 1 };
        }
        
        objToCamProj = objToCamProj.normalize();
        return computeAxisRotation(defaultAxis, objToCamProj).getMatrix().multiply(transform);
    }
}

VROQuaternion VROBillboardConstraint::computeAxisRotation(VROVector3f defaultAxis, VROVector3f objToCamProj) {
    // The direction the object is assumed to be facing (positive Z)
    VROVector3f lookAt(0, 0, 1);
    
    // Derives the axis and angle of billboard rotation
    VROVector3f axis = lookAt.cross(objToCamProj).normalize();
    float angleCosine = lookAt.dot(objToCamProj);
    
    // Use defaultAxis in edge cases because axis may be NaN
    if (angleCosine > 0.99999999) {
        return VROQuaternion::fromAngleAxis(0, defaultAxis);
    }
    else if (angleCosine < -0.99999999) {
        return VROQuaternion::fromAngleAxis(M_PI, defaultAxis);
    }
    else {
        return VROQuaternion::fromAngleAxis(acos(angleCosine), axis);
    }
}