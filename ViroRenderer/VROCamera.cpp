//
//  VROCamera.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

#include "VROCamera.h"
#include "VROLog.h"
#include "VROMath.h"

VROCamera::VROCamera() : _frustum() {
    
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
    VROMatrix4f rotation = _baseRotation.multiply(_headRotation);
    _rotation = { rotation };
    
    _forward = rotation.multiply(kBaseForward);
    _up = rotation.multiply(kBaseUp);
}

void VROCamera::computeLookAtMatrix() {
    _lookAtMatrix = VROMathComputeLookAtMatrix(_position, _forward, _up);
}

void VROCamera::computeFrustum() {
    /*
     The frustum does not have a far clipping plane, because we auto-adjust our
     FCP to fit all objects regardless of distance.
     */
    _frustum.fitToModelView(_lookAtMatrix.getArray(), _projection.getArray(), 0, 0, 0);
    _frustum.removeFCP();
}

void VROCamera::setViewport(VROViewport viewport) {
    _viewport = viewport;
}

void VROCamera::setFOV(VROFieldOfView fov) {
    _fov = fov;
}

void VROCamera::setProjection(VROMatrix4f projection) {
    _projection = projection;
    
    float c = projection[10];
    float d = projection[14];
    _ncp = d / (c - 1.0);
    _fcp = d / (c + 1.0);
}

float VROCamera::getWorldPerScreen(float distance) const {
    /*
     Arbitrarily chose eye's left FOV. tan(fov) = perp/distance, where
     perp is in the direction perpendicular to the camera's up vector and
     forward vector, and distance is in the direction of the camera's forward
     vector.
     */
    float radians = toRadians((_fov.getLeft() + _fov.getRight())) / 2.0;
    float perp = distance * tan(radians);
    
    /*
     The perspective divide is perp divided by half the viewport.
     */
    return perp / (_viewport.getWidth() / 2.0f);
}
