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

void VROCamera::setHeadRotation(VROMatrix4f headRotation) {
    _headRotation = headRotation;
    onRotationChanged();
}

void VROCamera::setBaseRotation(VROMatrix4f baseRotation) {
    _baseRotation = baseRotation;
    onRotationChanged();
}

void VROCamera::onRotationChanged() {
    VROMatrix4f rotation = _headRotation.multiply(_baseRotation);
    _rotation = { rotation };
    
    VROVector3f zAxis(0, 0, -1.0);
    _forward = rotation.multiply(zAxis);
    
    VROVector3f yAxis(0, 1.0, 0);
    _up = rotation.multiply(yAxis);
}

VROMatrix4f VROCamera::computeLookAtMatrix() const {
    return VROMathComputeLookAtMatrix(_position, _forward, _up);
}