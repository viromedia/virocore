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
    _A(A), _B(B) {
    
    _ABx = _B.x - _A.x;
    _ABy = _B.y - _A.y;
    _ABz = _B.z - _A.z;
    _lengthSq = _ABx * _ABx + _ABy * _ABy + _ABz * _ABz;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Basic Functions
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Basic Functions

VROVector3f VROLineSegment::getA() const {
    return _A;
}

VROVector3f VROLineSegment::getB() const {
    return _B;
}

float VROLineSegment::length() const {
    return sqrt(_lengthSq);
}

float VROLineSegment::lengthApprox() const {
    return VROMathFastSquareRoot(_lengthSq);
}

VROLineSegment VROLineSegment::flip() const {
    return { _B, _A };
}

VROVector3f VROLineSegment::ray() const {
    VROVector3f ray = _B.subtract(_A);
    return ray.normalize();
}

VROVector3f VROLineSegment::midpoint() const {
    VROVector3f midpoint = _A.add(_B);
    return midpoint.scale(0.5);
}

float VROLineSegment::parameterOfClosestPoint(VROVector3f p) const {
    double numerator = (p.x - _A.x) * _ABx + (p.y - _A.y) * _ABy + (p.z - _A.z) * _ABz;
    return numerator / _lengthSq;
}

VROVector3f VROLineSegment::pointOnLineSegmentClosestTo(VROVector3f p) const {
    float t = std::min(std::max(parameterOfClosestPoint(p), 0.0f), 1.0f);

    return { _A.x + _ABx * t,
             _A.y + _ABy * t,
             _A.z + _ABz * t };
}

VROVector3f VROLineSegment::pointOnLineClosestTo(VROVector3f p) const {
    float t = parameterOfClosestPoint(p);
    return { _A.x + _ABx * t,
             _A.y + _ABy * t,
             _A.z + _ABz * t };
}

float VROLineSegment::distanceToPoint(VROVector3f v) const {
    VROVector3f ptOnLine = pointOnLineSegmentClosestTo(v);
    return ptOnLine.distance(v);
}

float VROLineSegment::distanceToPointSigned(VROVector3f v) const {
    VROVector3f ptOnLine = pointOnLineSegmentClosestTo(v);

    double det = (_B.x - _A.x) * (v.y - _B.y) - (v.x - _B.x) * (_B.y - _A.y);
    if (det < 0) {
        return -ptOnLine.distance(v);
    } else {
        return ptOnLine.distance(v);
    }
}

std::string VROLineSegment::toString() const {
    std::stringstream ss;
    ss << "start [x: " << _A.x << ", y: " << _A.y << ", z: " << _A.z << "], end [x: " << _B.x << ", y: " << _B.y << ", z: " << _B.z << "]";

    return ss.str();
}

VROVector3f VROLineSegment::normal2DUnitVector(bool positive) const {
    double dx = _B.x - _A.x;
    double dy = _B.y - _A.y;

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

    VROVector3f A = _A;
    VROVector3f B = _B.rotateAboutAxis(zAxis, _A, radians);
    
    return { A, B };
}

VROLineSegment VROLineSegment::rotate(float radians) const {
    VROVector3f center = midpoint();
    VROVector3f zAxis(0, 0, 1);

    VROVector3f A = _A.rotateAboutAxis(zAxis, center, radians);
    VROVector3f B = _B.rotateAboutAxis(zAxis, center, radians);
    
    return { A, B };
}

VROLineSegment VROLineSegment::scale(float scale) const {
    VROVector3f m = midpoint();

    VROVector3f rayA = _A.subtract(m);
    VROVector3f rayB = _B.subtract(m);

    rayA = rayA.scale(scale);
    rayB = rayB.scale(scale);

    rayA = m.add(rayA);
    rayB = m.add(rayB);

    return { rayA, rayB };
}

void VROLineSegment::offsetByDistance(double distance, VROLineSegment *result) const {
    double dx = _B.x - _A.x;
    double dy = _B.y - _A.y;
    double len = VROMathFastSquareRoot(dx * dx + dy * dy);

    // u is the vector that is the length of the offset, in the direction of
    // the segment
    double ux = distance * dx / len;
    double uy = distance * dy / len;

    result->_A.x = _A.x - uy;
    result->_A.y = _A.y + ux;
    result->_B.x = _B.x - uy;
    result->_B.y = _B.y + ux;
}

/////////////////////////////////////////////////////////////////////////////////
//
// Angles
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Angles

VROOrientation VROLineSegment::orientationOfPoint(VROVector3f p) const {
    double det = (_B.x - _A.x) * (p.y - _B.y) - (p.x - _B.x) * (_B.y - _A.y);

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
    return { _A.subtract(r), _B.add(r) };
}

VROLineSegment VROLineSegment::extend(float amount) const {
    VROVector3f r = ray().scale(length() + amount);
    return { _A, _A.add(r) };
}

VROLineSegment VROLineSegment::extendBackwards(float amount) const {
    VROVector3f r = ray().scale(length() + amount);
    return { _B.subtract(r), _A };
}

VROVector3f VROLineSegment::traverseFromStart(float distance) const {
    return _A.add(ray().scale(distance));
}

VROVector3f VROLineSegment::traverseFromEnd(float distance) const {
    return _B.add(ray().scale(distance));
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Segment / Box Intersection
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Segment / Box Intersection

bool VROLineSegment::intersectsBox(float left, float right, float top, float bottom) const {
    VROLineSegment T({left, top}, {right, top});
    VROLineSegment R({right, top}, {right, bottom});
    VROLineSegment B({right, bottom}, {left, bottom});
    VROLineSegment L({left, bottom}, {left, top});

    bool result = intersectsSegment(T) || intersectsSegment(R) || intersectsSegment(L) || intersectsSegment(B);
    return result;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Segment / Segment Intersection
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Segment / Segment Intersection

bool VROLineSegment::intersectsSegment(VROLineSegment other) const {
    const VROVector3f *p0 = &_A;
    const VROVector3f *p1 = &_B;
    const VROVector3f *q0 = &other._A;
    const VROVector3f *q1 = &other._B;

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

bool VROLineSegment::intersectsSegment(VROLineSegment other, VROVector3f *result) const {
    const VROVector3f *p0 = &_A;
    const VROVector3f *p1 = &_B;
    VROVector3f *q0 = &other._A;
    VROVector3f *q1 = &other._B;

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

bool VROLineSegment::intersectsLine(VROLineSegment other, VROVector3f *result) const {
    const VROVector3f *p0 = &_A;
    const VROVector3f *p1 = &_B;
    VROVector3f *q0 = &other._A;
    VROVector3f *q1 = &other._B;

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
