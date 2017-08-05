//
//  VROPlane.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

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
