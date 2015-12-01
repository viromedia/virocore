//
//  VROMath.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROVector3f.h"
#include "VROMatrix4f.h"
#include "VROVector3d.h"
#include "VROMath.h"
#include <sstream>

VROVector3f::VROVector3f() {
    x = 0;
    y = 0;
    z = 0;
}

VROVector3f::VROVector3f(float xIn, float yIn) :
    x(xIn), y(yIn), z(0) {
}

VROVector3f::VROVector3f(float xIn, float yIn, float zIn) :
    x(xIn), y(yIn), z(zIn) {

}

VROVector3f::VROVector3f(const VROVector3f &vector) :
    x(vector.x),
    y(vector.y),
    z(vector.z) {
}

VROVector3f::VROVector3f(const VROVector3d &vector) :
    x((float) vector.x),
    y((float) vector.y),
    z((float) vector.z) {

}

VROVector3f::~VROVector3f() {

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

double VROVector3f::angleZ(const VROVector3f &other) const {
    VROVector3f v1(*this);
    VROVector3f v2(other);

    v1.z = 0;
    v2.z = 0;

    v1.normalize();
    v2.normalize();

    double dotProduct = v2.dot(v1);
    double magnitude = v1.magnitude() * v2.magnitude();
    double arcAngle = dotProduct / magnitude;

    double angle = acos(dotProduct / magnitude);
    if (arcAngle > 1) {
        angle = 0;
    }
    if (arcAngle < -1) {
        angle = M_PI;
    }

    VROVector3f cross;
    v1.cross(v2, &cross);

    if (cross.z <= 0) {
        return angle;
    } else {
        return angle * -1;
    }
}

float VROVector3f::angleZ_normed(const VROVector3f &other) const {
    double arcAngle = dot(other);

    double angle = 0;
    if (arcAngle > 1) {
        angle = 0;
    }
    else if (arcAngle < -1) {
        angle = M_PI;
    }
    else {
        angle = acos(arcAngle);
    }

    VROVector3f cross;
    this->cross(other, &cross);

    if (cross.z <= 0) {
        return angle;
    } else {
        return angle * -1;
    }
}

float VROVector3f::angleZ_xAxis() const {
    return atan2f(y, x);
}

float VROVector3f::angleWithNormedVector(const VROVector3f &vector) const {
    return acosf(dot(vector));
}

float VROVector3f::angleWithLine(const VROVector3f &line) const {
    float angle = acosf(dot(line));
    if (angle > M_PI_2) {
        angle = M_PI - angle;
    }
    return angle;
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

bool VROVector3f::lineIntersectPlane(const VROVector3f &point, const VROVector3f &normal,
                                     const VROVector3d &origin, VROVector3d *intPt) const {
    double denom = dot(normal);
    if (denom == 0) {
        return false;
    }

    float c = normal.dot(point);
    double t = (c - normal.dot(origin)) / denom;

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

bool VROVector3f::rayIntersectPlane(const VROVector3f &point, const VROVector3f &normal,
                                    const VROVector3d &origin, VROVector3d *intPt) const {
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

void VROVector3f::rotateZ(float angleRad, VROVector3f *result) const {
    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    float sinR = sincosr[0];
    float cosR = sincosr[1];

    float rx = x * cosR - y * sinR;
    float ry = x * sinR + y * cosR;

    result->x = rx;
    result->y = ry;
}

void VROVector3f::rotateAboutAxis(const VROVector3f &axisDir, const VROVector3f &axisPos,
                                  float angleRad, VROVector3f *result) const {
    VROMatrix4f pivot;
    pivot.rotate(angleRad, axisPos, axisDir);
    *result = pivot.multiply(*this);
}

float VROVector3f::distanceXY(const VROVector3f &vector) const  {
    return sqrt((vector.y - y) * (vector.y - y) + (vector.x - x) * (vector.x - x));
}

void VROVector3f::set(const VROVector3f &value) {
    x = value.x;
    y = value.y;
    z = value.z;
}

void VROVector3f::set(const VROVector3d &value) {
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

double VROVector3f::dot(const VROVector3d &vB) const {
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

float VROVector3f::distance(const VROVector3d &vB) const  {
    float dx = (vB.x - this->x);
    float dy = (vB.y - this->y);
    float dz = (vB.z - this->z);

    return VROMathFastSquareRoot(dx * dx + dy * dy + dz * dz);
}

float VROVector3f::distanceSquared(const VROVector3f &vB) const {
    float dx = (vB.x - this->x);
    float dy = (vB.y - this->y);
    float dz = (vB.z - this->z);

    return dx * dx + dy * dy + dz * dz;
}

void VROVector3f::add(const VROVector3f &vB, VROVector3f *result) const {
    result->x = x + vB.x;
    result->y = y + vB.y;
    result->z = z + vB.z;
}

void VROVector3f::addScaled(const VROVector3f &scaledB, float scale, VROVector3f *result) const {
    result->x = x + scaledB.x * scale;
    result->y = y + scaledB.y * scale;
    result->z = z + scaledB.z * scale;
}

void VROVector3f::subtract(const VROVector3f &vB, VROVector3f *result) const {
    result->x = x - vB.x;
    result->y = y - vB.y;
    result->z = z - vB.z;
}

void VROVector3f::midpoint(const VROVector3f &other, VROVector3f *result) const {
    result->x = (x + other.x) * 0.5f;
    result->y = (y + other.y) * 0.5f;
    result->z = (z + other.z) * 0.5f;
}

void VROVector3f::midpoint(const VROVector3d &other, VROVector3f *result) const {
    result->x = (x + other.x) * 0.5;
    result->y = (y + other.y) * 0.5;
    result->z = (z + other.z) * 0.5;
}

void VROVector3f::cross(const VROVector3f &vB, VROVector3f *result) const {
    result->x = y * vB.z - z * vB.y;
    result->y = z * vB.x - x * vB.z;
    result->z = x * vB.y - y * vB.x;
}

VROVector3f VROVector3f::cross(const VROVector3f &vB) const {
    VROVector3f result;
    result.x = y * vB.z - z * vB.y;
    result.y = z * vB.x - x * vB.z;
    result.z = x * vB.y - y * vB.x;
    
    return result;
}

void VROVector3f::normalize() {
    float inverseMag = 1.0f / sqrtf(x * x + y * y + z * z);

    this->x *= inverseMag;
    this->y *= inverseMag;
    this->z *= inverseMag;
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

void VROVector3f::scale(float factor, VROVector3f *result) const {
    result->x = x * factor;
    result->y = y * factor;
    result->z = z * factor;
}

std::string VROVector3f::toString() const {
    std::stringstream ss;
    ss << "[x: " << x << ", y: " << y << ", z: " << z << "]";

    return ss.str();
}
