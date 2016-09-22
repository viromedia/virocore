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
    
    _forward = rotation.multiply(kBaseForward);
    _up = rotation.multiply(kBaseUp);
}

VROMatrix4f VROCamera::computeLookAtMatrix() const {
    return VROMathComputeLookAtMatrix(_position, _forward, _up);
}