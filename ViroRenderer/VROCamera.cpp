//
//  VROCamera.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROCamera.h"
#include "VROLog.h"
#include "VROMath.h"

VROCamera::VROCamera() {
    
}

VROCamera::~VROCamera() {
    
}

void VROCamera::setPosition(VROVector3f position) {
    _position = position;
}

void VROCamera::setHeadRotation(VROQuaternion headRotation) {
    _headRotation = headRotation;
    onRotationChanged();
}

void VROCamera::setBaseRotation(VROQuaternion baseRotation) {
    _baseRotation = baseRotation;
    onRotationChanged();
}

void VROCamera::onRotationChanged() {
    _rotation = _headRotation * _baseRotation;
    
    VROVector3f zAxis(0, 0, -1.0);
    _forward = _rotation * zAxis;
    
    VROVector3f yAxis(0, 1.0, 0);
    _up = _rotation * yAxis;
}

VROMatrix4f VROCamera::computeLookAtMatrix() const {
    return VROMathComputeLookAtMatrix(_position, _forward, _up);
}