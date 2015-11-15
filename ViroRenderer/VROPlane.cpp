//
//  VROPlane.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROPlane.h"
#include "VROVector3f.h"
#include "VROVector3d.h"

VROPlane::VROPlane() :
    d(0) {

}

VROPlane::VROPlane(const VROVector3f &normal, float d) :
    normal(normal),
    d(d) {
    
}

VROPlane::~VROPlane() {

}

void VROPlane::projectNormalizedVector(const VROVector3f &vector, VROVector3f *result) {
    VROVector3f cross;
    vector.cross(normal, &cross);
    normal.cross(cross, result);
}

float VROPlane::distanceToPoint(const VROVector3f &point) const {
    return normal.dot(point) + d;
}

float VROPlane::distanceToPointXY(const VROVector3f &point) const {
    return normal.x * point.x + normal.y * point.y + d;
}

float VROPlane::distanceToPoint(const VROVector3d &point) const {
    return normal.dot(point) + d;
}

float VROPlane::distanceToPointXY(const VROVector3d &point) const {
    return normal.x * point.x + normal.y * point.y + d;
}

int VROPlane::getHalfSpaceOfPoint(const VROVector3f &point) const {
    float distance = distanceToPoint(point);
    if (distance < 0) {
        return NEGATIVE_HALF_SPACE;
    }
    if (distance > 0) {
        return POSITIVE_HALF_SPACE;
    }
    return ON_PLANE;
}

void VROPlane::normalize() {
    float mag = normal.magnitude();
    normal.normalize();

    d /= mag;
}
