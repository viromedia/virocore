//
//  VROPlane.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
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

#include "VROPlane.h"
#include "VROVector3f.h"

VROPlane::VROPlane() :
    d(0) {
}

VROPlane::VROPlane(VROVector3f normal, float d) :
    normal(normal),
    d(d) {
}

VROPlane::VROPlane(VROVector3f normal_in, VROVector3f point) :
    normal(normal_in),
    d(-normal_in.dot(point)) {
}

VROPlane::~VROPlane() {

}

VROVector3f VROPlane::projectNormalizedVector(VROVector3f vector) {
    VROVector3f cross = vector.cross(normal);
    return normal.cross(cross);
}

float VROPlane::distanceToPoint(VROVector3f point) const {
    return normal.dot(point) + d;
}

float VROPlane::distanceToPointXY(VROVector3f point) const {
    return normal.x * point.x + normal.y * point.y + d;
}

VROPlaneHalfSpace VROPlane::getHalfSpaceOfPoint(VROVector3f point) const {
    float distance = distanceToPoint(point);
    if (distance < 0) {
        return VROPlaneHalfSpace::Negative;
    }
    if (distance > 0) {
        return VROPlaneHalfSpace::Positive;
    }
    return VROPlaneHalfSpace::OnPlane;
}

void VROPlane::normalize() {
    float mag = normal.magnitude();
    normal = normal.normalize();

    d /= mag;
}
