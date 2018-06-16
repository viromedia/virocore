//
//  VRONodeCamera.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VRONodeCamera.h"
#include "VROAnimationFloat.h"
#include "VROAnimationVector3f.h"
#include "VROAnimationQuaternion.h"
#include "VROCamera.h"

VRONodeCamera::VRONodeCamera() :
    _rotationType(VROCameraRotationType::Standard),
    _fov(0) {
    
}

VRONodeCamera::~VRONodeCamera() {
    
}

void VRONodeCamera::setPosition(VROVector3f position) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f p) {
        ((VRONodeCamera *)animatable)->_position = p;
    }, _position, position));
}

void VRONodeCamera::setBaseRotation(VROQuaternion baseRotation) {
    animate(std::make_shared<VROAnimationQuaternion>([](VROAnimatable *const animatable, VROQuaternion r) {
        ((VRONodeCamera *)animatable)->_baseRotation = r;
    }, _baseRotation, baseRotation));
}

void VRONodeCamera::setRotationType(VROCameraRotationType type) {
    _rotationType = type;
}

void VRONodeCamera::setOrbitFocalPoint(VROVector3f focalPt) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f o) {
        ((VRONodeCamera *)animatable)->_orbitFocalPt = o;
    }, _orbitFocalPt, focalPt));
}

void VRONodeCamera::setFieldOfViewY(float fovY) {
    animate(std::make_shared<VROAnimationFloat>([](VROAnimatable *const animatable, float f) {
        ((VRONodeCamera *)animatable)->_fov = f;
    }, _fov, fovY));
}

void VRONodeCamera::setRefNodeToCopyRotation(std::shared_ptr<VRONode> node) {
    _refNodeToCopyCameraRotation = node;
}

