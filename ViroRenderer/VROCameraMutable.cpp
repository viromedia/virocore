//
//  VROCameraMutable.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROCameraMutable.h"
#include "VROAnimationVector3f.h"
#include "VROAnimationQuaternion.h"

VROCameraMutable::VROCameraMutable() :
    _rotationType(VROCameraRotationType::Standard) {
    
}

VROCameraMutable::~VROCameraMutable() {
    
}

void VROCameraMutable::setPosition(VROVector3f position) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f p) {
        ((VROCameraMutable *)animatable)->_position = p;
    }, _position, position));
}

void VROCameraMutable::setBaseRotation(VROQuaternion baseRotation) {
    animate(std::make_shared<VROAnimationQuaternion>([](VROAnimatable *const animatable, VROQuaternion r) {
        ((VROCameraMutable *)animatable)->_baseRotation = r;
    }, _baseRotation, baseRotation));
}

void VROCameraMutable::setRotationType(VROCameraRotationType type) {
    _rotationType = type;
}

void VROCameraMutable::setOrbitFocalPoint(VROVector3f focalPt) {
    animate(std::make_shared<VROAnimationVector3f>([](VROAnimatable *const animatable, VROVector3f o) {
        ((VROCameraMutable *)animatable)->_orbitFocalPt = o;
    }, _orbitFocalPt, focalPt));
}