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
    
    VROMatrix4f rotation = transform;
    rotation[12] = 0;
    rotation[13] = 0;
    rotation[14] = 0;
    
    // The object is assumed to start facing in the positive Z
    // direction; we rotate it by the transform to get the current
    // lookAt vector
    VROVector3f lookAt(0, 0, 1);
    lookAt = rotation.multiply(lookAt).normalize();
    
    const VROCamera &camera = context.getCamera();
    
    if (_freeAxis == VROBillboardAxis::All) {
        // Billboard with free Y axis
        VROVector3f objToCam = camera.getPosition().subtract(node.getComputedPosition());
        
        VROVector3f objToCamProj = objToCam;
        objToCamProj.y = 0;
        objToCamProj = objToCamProj.normalize();

        VROQuaternion quaternionY = computeAxisRotation(lookAt, { 0, 1, 0 }, objToCamProj);
        
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
        return composed.getMatrix();
    }
    else {
        VROVector3f objToCamProj = camera.getPosition().subtract(node.getComputedPosition());
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
        
        VROQuaternion composed = computeAxisRotation(lookAt, defaultAxis, objToCamProj);
        return composed.getMatrix();
    }
}

VROQuaternion VROBillboardConstraint::computeAxisRotation(VROVector3f lookAt, VROVector3f defaultAxis,
                                                          VROVector3f objToCamProj) {
    
    
    // Derive the axis and angle of billboard rotation.
    // The axis is the cross product of the vector from the object
    // to the camera with the object's look-at vector, and the angle
    // is derived from the dot product, since dot(A,B)=cos(angle) for
    // normalized vectors.
    VROVector3f axis = lookAt.cross(objToCamProj).normalize();
    float angleCosine = lookAt.dot(objToCamProj);
    
    // Use defaultAxis in edge cases because axis may be NaN
    if (angleCosine > 0.99999999) {
        return VROQuaternion::fromAngleAxis(0, defaultAxis);
    }
    else if (angleCosine < -0.99999999) {
        return VROQuaternion::fromAngleAxis(M_PI, defaultAxis);
    }
    else if (isnan(axis.x) || isnan(axis.y) || isnan(axis.z)) {
        return VROQuaternion::fromAngleAxis(0, defaultAxis);
    }
    else {
        return VROQuaternion::fromAngleAxis(acos(angleCosine), axis);
    }
}
