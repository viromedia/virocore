//
//  VROMath.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include "VROMath.h"
#include <sstream>

VROVector3f::VROVector3f() noexcept : x(0), y(0), z(0) {
}

VROVector3f::VROVector3f(float xIn, float yIn) :
    x(xIn), y(yIn), z(0) {
}

VROVector3f::VROVector3f(float xIn, float yIn, float zIn) :
    x(xIn), y(yIn), z(zIn) {

}

int VROVector3f::hash() const {
    return floor(x + 31 * y + 31 * z);
}

bool VROVector3f::isEqual(const VROVector3f &vertex) const {
    return fabs(x - vertex.x) < .00001 && fabs(y - vertex.y) < .00001 && fabs(z - vertex.z) < .00001;
}

void VROVector3f::clear() {
    x = 0;
    y = 0;
    z = 0;
}

float VROVector3f::angleWithNormedVector(const VROVector3f &vector) const {
    return acos(VROMathClamp(dot(vector), -1.0 + kEpsilon, 1.0 - kEpsilon));
}

float VROVector3f::angleWithVector(const VROVector3f &vector) const {
    float lenSq1 = x * x + y * y + z * z;
    float lenSq2 = vector.x * vector.x + vector.y * vector.y + vector.z * vector.z;
    
    return acos(VROMathClamp(dot(vector)/sqrt(lenSq1 * lenSq2), -1.0 + kEpsilon, 1.0 - kEpsilon));
}

bool VROVector3f::lineIntersectPlane(const VROVector3f &point, const VROVector3f &normal,
                                     const VROVector3f &origin, VROVector3f *intPt) const {
    float denom = dot(normal);
    if (denom == 0) {
        return false;
    }

    float c = normal.dot(point);
    float t = (c - normal.dot(origin)) / denom;

    intPt->x = origin.x + x * t;
    intPt->y = origin.y + y * t;
    intPt->z = origin.z + z * t;

    return true;
}

bool VROVector3f::rayIntersectPlane(const VROVector3f &point, const VROVector3f &normal,
                                    const VROVector3f &origin, VROVector3f *intPt) const {
    float denom = dot(normal);
    if (denom == 0) {
        return false;
    }

    float c = normal.dot(point);
    float t = (c - normal.dot(origin)) / denom;
    if (t < 0) {
        return false;
    }

    intPt->x = origin.x + x * t;
    intPt->y = origin.y + y * t;
    intPt->z = origin.z + z * t;

    return true;
}

bool VROVector3f::projectOnPlane(const VROVector3f &point, const VROVector3f &normal, VROVector3f *projPoint) {
    VROVector3f v = subtract(point);

    VROVector3f normalizedNormal = normal.normalize();

    float dist = v.dot(normalizedNormal);
    if (dist < 0) {
        return false;
    }

    projPoint->x = x - dist * normalizedNormal.x;
    projPoint->y = y - dist * normalizedNormal.y;
    projPoint->z = z - dist * normalizedNormal.z;

    return true;
}

VROVector3f VROVector3f::rotateZ(float angleRad) const {
    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    float sinR = sincosr[0];
    float cosR = sincosr[1];
    float rx = x * cosR - y * sinR;
    float ry = x * sinR + y * cosR;

    VROVector3f result;
    result.x = rx;
    result.y = ry;
    
    return result;
}

VROVector3f VROVector3f::rotateAboutAxis(const VROVector3f &axisDir, const VROVector3f &axisPos,
                                  float angleRad) const {
    VROMatrix4f pivot;
    pivot.rotate(angleRad, axisPos, axisDir);
    
    return pivot.multiply(*this);
}

float VROVector3f::distanceXY(const VROVector3f &vector) const  {
    return sqrt((vector.y - y) * (vector.y - y) + (vector.x - x) * (vector.x - x));
}

void VROVector3f::set(const VROVector3f &value) {
    x = value.x;
    y = value.y;
    z = value.z;
}

void VROVector3f::set(float x, float y, float z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

float VROVector3f::dot(const VROVector3f &vB) const {
    return x * vB.x + y * vB.y + z * vB.z;
}

/**
 * WARNING: this will return NaN for values that are the same!
 */
float VROVector3f::distance(const VROVector3f &vB) const {
    float dx = (vB.x - this->x);
    float dy = (vB.y - this->y);
    float dz = (vB.z - this->z);

    return VROMathFastSquareRoot(dx * dx + dy * dy + dz * dz);
}

float VROVector3f::distanceAccurate(const VROVector3f &vB) const {
    float dx = (vB.x - this->x);
    float dy = (vB.y - this->y);
    float dz = (vB.z - this->z);

    return (float)sqrt(dx * dx + dy * dy + dz * dz);
}

float VROVector3f::distanceSquared(const VROVector3f &vB) const {
    float dx = (vB.x - this->x);
    float dy = (vB.y - this->y);
    float dz = (vB.z - this->z);

    return dx * dx + dy * dy + dz * dz;
}

VROVector3f VROVector3f::add(VROVector3f vB) const {
    VROVector3f result;
    result.x = x + vB.x;
    result.y = y + vB.y;
    result.z = z + vB.z;
    
    return result;
}

VROVector3f VROVector3f::addScaled(const VROVector3f &scaledB, float scale) const {
    VROVector3f result;
    result.x = x + scaledB.x * scale;
    result.y = y + scaledB.y * scale;
    result.z = z + scaledB.z * scale;
    
    return result;
}

VROVector3f VROVector3f::subtract(VROVector3f vB) const {
    VROVector3f result;
    result.x = x - vB.x;
    result.y = y - vB.y;
    result.z = z - vB.z;
    
    return result;
}

VROVector3f VROVector3f::midpoint(const VROVector3f &other) const {
    VROVector3f result;
    result.x = (x + other.x) * 0.5f;
    result.y = (y + other.y) * 0.5f;
    result.z = (z + other.z) * 0.5f;
    
    return result;
}

VROVector3f VROVector3f::cross(const VROVector3f &vB) const {
    VROVector3f result;
    result.x = y * vB.z - z * vB.y;
    result.y = z * vB.x - x * vB.z;
    result.z = x * vB.y - y * vB.x;
    
    return result;
}

VROVector3f VROVector3f::normalize() const {
    float inverseMag = 1.0f / sqrtf(x * x + y * y + z * z);

    VROVector3f result;
    result.x = x * inverseMag;
    result.y = y * inverseMag;
    result.z = z * inverseMag;
    
    return result;
}

float VROVector3f::magnitude() const {
    return sqrt(x * x + y * y + z * z);
}

bool VROVector3f::isZero() const {
    return x == 0 && y == 0 && z == 0;
}

float VROVector3f::magnitudeXY() const {
    return sqrt(x * x + y * y);
}

VROVector3f VROVector3f::scale(float factor) const {
    VROVector3f result;
    result.x = x * factor;
    result.y = y * factor;
    result.z = z * factor;
    
    return result;
}

VROVector3f VROVector3f::interpolate(VROVector3f other, float t) const {
    VROVector3f result;
    result.x = x + (other.x - x) * t;
    result.y = y + (other.y - y) * t;
    result.z = z + (other.z - z) * t;

    return result;
}

std::string VROVector3f::toString() const {
    std::stringstream ss;
    ss << "[x: " << x << ", y: " << y << ", z: " << z << "]";

    return ss.str();
}

void VROVector3f::toArray(float *array) const {
    array[0] = x;
    array[1] = y;
    array[2] = z;
}
