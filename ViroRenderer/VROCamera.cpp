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

void VROCamera::setViewport(VROViewport viewport) {
    _viewport = viewport;
}

void VROCamera::setFOV(VROFieldOfView fov) {
    _fov = fov;
}

float VROCamera::getWorldPerScreen(float distance) const {
    /*
     Arbitrarily chose eye's left FOV. tan(fov) = perp/distance, where
     perp is in the direction perpendicular to the camera's up vector and
     forward vector, and distance is in the direction of the camera's forward
     vector.
     */
    float radians = _fov.getLeft();
    float perp = distance * tan(radians);
    
    /*
     The perspective divide is perp divided by half the viewport.
     */
    return perp / (_viewport.getWidth() / 2.0f);
}
