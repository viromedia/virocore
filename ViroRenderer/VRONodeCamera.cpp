//
//  VRONodeCamera.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/24/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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

