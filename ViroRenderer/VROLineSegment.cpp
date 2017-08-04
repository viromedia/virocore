//
//  VROTriangle.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/12/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROLineSegment.h"
#include "VROVector3f.h"
#include "VROMath.h"
#include "VROBoundingBox.h"
#include "VROMath.h"
#include <algorithm>
#include <iostream>
#include <sstream>

/////////////////////////////////////////////////////////////////////////////////
//
//  Initialization
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Initialization

VROLineSegment::VROLineSegment(VROVector3f A, VROVector3f B) :
    __A(A), __B(B) {
    
    __ABx = __B.x - __A.x;
    __ABy = __B.y - __A.y;
    __ABz = __B.z - __A.z;
    _lengthSq = __ABx * __ABx + __ABy * __ABy + __ABz * __ABz;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Basic Functions
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Basic Functions

VROVector3f VROLineSegment::getA() const {
    return __A;
}

VROVector3f VROLineSegment::getB() const {
    return __B;
}

float VROLineSegment::length() const {
    return sqrt(_lengthSq);
}

float VROLineSegment::lengthApprox() const {
    return VROMathFastSquareRoot(_lengthSq);
}

VROLineSegment VROLineSegment::flip() const {
    return { __B, __A };
}

VROVector3f VROLineSegment::ray() const {
    VROVector3f ray = __B.subtract(__A);
    return ray.normalize();
}

VROVector3f VROLineSegment::midpoint() const {
    VROVector3f midpoint = __A.add(__B);
    return midpoint.scale(0.5);
}

float VROLineSegment::parameterOfClosestPoint(VROVector3f p) const {
    double numerator = (p.x - __A.x) * __ABx + (p.y - __A.y) * __ABy + (p.z - __A.z) * __ABz;
    return numerator / _lengthSq;
}

VROVector3f VROLineSegment::pointOnLineSegmentClosestTo(VROVector3f p) const {
    float t = std::min(std::max(parameterOfClosestPoint(p), 0.0f), 1.0f);

    return { __A.x + __ABx * t,
             __A.y + __ABy * t,
             __A.z + __ABz * t };
}

VROVector3f VROLineSegment::pointOnLineClosestTo(VROVector3f p) const {
    float t = parameterOfClosestPoint(p);
    return { __A.x + __ABx * t,
             __A.y + __ABy * t,
             __A.z + __ABz * t };
}

float VROLineSegment::distanceToPoint(VROVector3f v) const {
    VROVector3f ptOnLine = pointOnLineSegmentClosestTo(v);
    return ptOnLine.distance(v);
}

float VROLineSegment::distanceToPointSigned(VROVector3f v) const {
    VROVector3f ptOnLine = pointOnLineSegmentClosestTo(v);

    double det = (__B.x - __A.x) * (v.y - __B.y) - (v.x - __B.x) * (__B.y - __A.y);
    if (det < 0) {
        return -ptOnLine.distance(v);
    } else {
        return ptOnLine.distance(v);
    }
}

std::string VROLineSegment::toString() const {
    std::stringstream ss;
    ss << "start [x: " << __A.x << ", y: " << __A.y << ", z: " << __A.z << "], end [x: " << __B.x << ", y: " << __B.y << ", z: " << __B.z << "]";

    return ss.str();
}

VROVector3f VROLineSegment::normal2DUnitVector(bool positive) const {
    double dx = __B.x - __A.x;
    double dy = __B.y - __A.y;

    // normalize
    double length = VROMathFastSquareRoot(dx * dx + dy * dy);
    if (length == 0.0) {
        return VROVector3f(0, 0);
    }

    dx /= length;
    dy /= length;

    return positive ? VROVector3f(-dy, dx) : VROVector3f(dy, -dx);
}

/////////////////////////////////////////////////////////////////////////////////
//
// Basic Transforms
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Basic Transforms

VROLineSegment VROLineSegment::pivot(float radians) const {
    VROVector3f zAxis(0, 0, 1);

    VROVector3f A = __A;
    VROVector3f B = __B.rotateAboutAxis(zAxis, __A, radians);
    
    return { A, B };
}

VROLineSegment VROLineSegment::rotate(float radians) const {
    VROVector3f center = midpoint();
    VROVector3f zAxis(0, 0, 1);

    VROVector3f A = __A.rotateAboutAxis(zAxis, center, radians);
    VROVector3f B = __B.rotateAboutAxis(zAxis, center, radians);
    
    return { A, B };
}

VROLineSegment VROLineSegment::scale(float scale) const {
    VROVector3f m = midpoint();

    VROVector3f rayA = __A.subtract(m);
    VROVector3f rayB = __B.subtract(m);

    rayA = rayA.scale(scale);
    rayB = rayB.scale(scale);

    rayA = m.add(rayA);
    rayB = m.add(rayB);

    return { rayA, rayB };
}

void VROLineSegment::offsetByDistance(double distance, VROLineSegment *result) const {
    double dx = __B.x - __A.x;
    double dy = __B.y - __A.y;
    double len = VROMathFastSquareRoot(dx * dx + dy * dy);

    // u is the vector that is the length of the offset, in the direction of
    // the segment
    double ux = distance * dx / len;
    double uy = distance * dy / len;

    result->__A.x = __A.x - uy;
    result->__A.y = __A.y + ux;
    result->__B.x = __B.x - uy;
    result->__B.y = __B.y + ux;
}

/////////////////////////////////////////////////////////////////////////////////
//
// Angles
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Angles

VROOrientation VROLineSegment::orientationOfPoint(VROVector3f p) const {
    double det = (__B.x - __A.x) * (p.y - __B.y) - (p.x - __B.x) * (__B.y - __A.y);

    if (det > 0) {
        return VROOrientation::Right;
    }
    else if (det < 0) {
        return VROOrientation::Left;
    }
    else {
        return VROOrientation::Colinear;
    }
}

float VROLineSegment::angleWithSegment(VROLineSegment other) const {
    VROVector3f u = ray();
    VROVector3f v = other.ray();

    float angle = u.angleWithNormedVector(v);
    if (angle > M_PI_2) {
        angle = M_PI - angle;
    }
    if (isnan(angle)) {
        angle = 0;
    }

    return angle;
}

float VROLineSegment::angle2DWithSegment(VROLineSegment other) const {
    VROVector3f u = ray();
    u.z = 0;

    VROVector3f v = other.ray();
    v.z = 0;

    float angle = u.angleWithNormedVector(v);
    if (angle > M_PI_2) {
        angle = M_PI - angle;
    }
    if (isnan(angle)) {
        angle = 0;
    }

    return angle;
}

float VROLineSegment::directedAngleWithSegment(VROLineSegment other) const {
    VROVector3f u = ray();
    VROVector3f v = other.ray();

    float angle = u.angleWithNormedVector(v);
    if (isnan(angle)) {
        angle = 0;
    }

    return -angle;
}

float VROLineSegment::directedAngleWithRay(VROVector3f r) const {
    float angle = ray().angleWithNormedVector(r);
    if (isnan(angle)) {
        angle = 0;
    }

    return -angle;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Traverse and Extend
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Traverse and Extend

VROLineSegment VROLineSegment::extendForwardAndBackward(float amount) const {
    VROVector3f r = ray().scale(amount);
    return { __A.subtract(r), __B.add(r) };
}

VROLineSegment VROLineSegment::extend(float amount) const {
    VROVector3f r = ray().scale(length() + amount);
    return { __A, __A.add(r) };
}

VROLineSegment VROLineSegment::extendBackwards(float amount) const {
    VROVector3f r = ray().scale(length() + amount);
    return { __B.subtract(r), __A };
}

VROVector3f VROLineSegment::traverseFromStart(float distance) const {
    return __A.add(ray().scale(distance));
}

VROVector3f VROLineSegment::traverseFromEnd(float distance) const {
    return __B.add(ray().scale(distance));
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Segment / Box Intersection
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Segment / Box Intersection

bool VROLineSegment::intersectsBox2D(float left, float right, float top, float bottom) const {
    VROLineSegment T({left, top}, {right, top});
    VROLineSegment R({right, top}, {right, bottom});
    VROLineSegment B({right, bottom}, {left, bottom});
    VROLineSegment L({left, bottom}, {left, top});

    bool result = intersectsSegment2D(T) || intersectsSegment2D(R) || intersectsSegment2D(L) || intersectsSegment2D(B);
    return result;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Segment / Segment Intersection
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Segment / Segment Intersection

bool VROLineSegment::intersectsSegment2D(VROLineSegment other) const {
    const VROVector3f *p0 = &__A;
    const VROVector3f *p1 = &__B;
    const VROVector3f *q0 = &other.__A;
    const VROVector3f *q1 = &other.__B;

    float pMx = p1->x - p0->x;
    float pMy = p1->y - p0->y;
    float qMx = q1->x - q0->x;
    float qMy = q1->y - q0->y;

    float denom = -qMx * pMy + pMx * qMy;
    if (denom == 0) {
        return false; //parallel
    }

    float s = (-pMy * (p0->x - q0->x) + pMx * (p0->y - q0->y)) / denom;
    float t =  (qMx * (p0->y - q0->y) - qMy * (p0->x - q0->x)) / denom;

    return (s >= 0 && s <= 1 && t >= 0 && t <= 1);
}

bool VROLineSegment::intersectsSegment2D(VROLineSegment other, VROVector3f *result) const {
    const VROVector3f *p0 = &__A;
    const VROVector3f *p1 = &__B;
    VROVector3f *q0 = &other.__A;
    VROVector3f *q1 = &other.__B;

    float pMx = p1->x - p0->x;
    float pMy = p1->y - p0->y;
    float qMx = q1->x - q0->x;
    float qMy = q1->y - q0->y;

    float denom = -qMx * pMy + pMx * qMy;
    if (denom == 0) {
        return false; //parallel
    }

    float s = (-pMy * (p0->x - q0->x) + pMx * (p0->y - q0->y)) / denom;
    float t =  (qMx * (p0->y - q0->y) - qMy * (p0->x - q0->x)) / denom;

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        result->x = p0->x + (t * pMx);
        result->y = p0->y + (t * pMy);
        result->z = 0;
        
        return true;
    }
    return false;
}

bool VROLineSegment::intersectsPlane(VROVector3f point, VROVector3f normal, VROVector3f *outIntersectionPoint) const {
    VROVector3f r = __B - __A;
    float denom = r.dot(normal);
    if (denom == 0) {
        return false;
    }
    
    float c = normal.dot(point);
    float t = (c - normal.dot(__A)) / denom;
    if (t < 0 || t > 1) {
        return false;
    }
    
    outIntersectionPoint->x = __A.x + r.x * t;
    outIntersectionPoint->y = __A.y + r.y * t;
    outIntersectionPoint->z = __A.z + r.z * t;
    return true;
}

bool VROLineSegment::intersectsLine(VROLineSegment other, VROVector3f *result) const {
    const VROVector3f *p0 = &__A;
    const VROVector3f *p1 = &__B;
    VROVector3f *q0 = &other.__A;
    VROVector3f *q1 = &other.__B;

    float pMx = p1->x - p0->x;
    float pMy = p1->y - p0->y;
    float qMx = q1->x - q0->x;
    float qMy = q1->y - q0->y;

    float denom = -qMx * pMy + pMx * qMy;
    if (denom == 0) {
        return false; //parallel
    }

    float t =  (qMx * (p0->y - q0->y) - qMy * (p0->x - q0->x)) / denom;

    result->x = p0->x + (t * pMx);
    result->y = p0->y + (t * pMy);
    result->z = 0;

    return true;
}
