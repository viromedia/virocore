//
//  VROVector3d.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROVector3d.h"
#include "VROMatrix4f.h"
#include "VROVector3f.h"
#include "VROMatrix4d.h"
#include "VROMath.h"
#include <sstream>

VROVector3d::VROVector3d() :
    x(0), y(0), z(0) {

}

VROVector3d::VROVector3d(double xIn, double yIn) :
    x(xIn), y(yIn), z(0) {

}

VROVector3d::VROVector3d(double xIn, double yIn, double zIn) :
    x(xIn), y(yIn), z(zIn) {
}

VROVector3d::VROVector3d(VROVector3d const &vector) :
    x(vector.x),
    y(vector.y),
    z(vector.z) {
}

VROVector3d::VROVector3d(VROVector3f const &vector) :
    x(vector.x),
    y(vector.y),
    z(vector.z) {
}

int VROVector3d::hash() const {
    return floor(x + 31 * y + 31 * z);
}

bool VROVector3d::isEqual(const VROVector3d &vertex) const {
    return fabs(x - vertex.x) < .00001 && fabs(y - vertex.y) < .00001 && fabs(z - vertex.z) < .00001;
}

void VROVector3d::clear() {
    x = 0;
    y = 0;
    z = 0;
}

double VROVector3d::angleZ(const VROVector3d &other) const {
    VROVector3d v1(*this);
    VROVector3d v2(other);

    v1.normalize();
    v2.normalize();

    v1.z = 0;
    v2.z = 0;

    double dotProduct = v2.dot(v1);
    double magnitude = v1.magnitude() * v2.magnitude();
    double arcAngle = dotProduct / magnitude;

    double angle;
    if (arcAngle > 1) {
        angle = 0;
    } else if (arcAngle < -1) {
        angle = M_PI;
    } else {
        angle = acos(arcAngle);
    }

    VROVector3d cross;
    v1.cross(v2, &cross);

    if (cross.z <= 0) {
        return angle;
    }
    else {
        return angle * -1;
    }
}

double VROVector3d::angleWithNormedVector(const VROVector3d &vector) const {
    return acosf(dot(vector));
}

double VROVector3d::angleWithLine(const VROVector3d &line) const {
    double angle = acosf(dot(line));
    if (angle > M_PI_2) {
        angle = M_PI - angle;
    }
    return angle;
}

bool VROVector3d::lineIntersectPlane(const VROVector3d &point, const VROVector3d &normal,
                                     const VROVector3d &origin, VROVector3d *intPt) const {
    double denom = dot(normal);
    if (denom == 0) {
        return false;
    }

    double c = normal.dot(point);
    double t = (c - normal.dot(origin)) / denom;

    intPt->x = origin.x + x * t;
    intPt->y = origin.y + y * t;
    intPt->z = origin.z + z * t;

    return true;
}

bool VROVector3d::rayIntersectPlane(const VROVector3d &point, const VROVector3d &normal, const VROVector3d &origin,
                                    VROVector3d *intPt) const {
    double denom = dot(normal);
    if (denom == 0) {
        return false;
    }

    double c = normal.dot(point);
    double t = (c - normal.dot(origin)) / denom;
    if (t < 0) {
        return false;
    }

    intPt->x = origin.x + x * t;
    intPt->y = origin.y + y * t;
    intPt->z = origin.z + z * t;

    return true;
}

void VROVector3d::rotateZ(float angleRad, VROVector3f *result) const {
    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    double sinR = sincosr[0];
    double cosR = sincosr[1];

    double rx = x * cosR - y * sinR;
    double ry = x * sinR + y * cosR;

    result->x = rx;
    result->y = ry;
}

void VROVector3d::rotateZ(double angleRad, VROVector3d *result) const {
    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    double sinR = sincosr[0];
    double cosR = sincosr[1];

    double rx = x * cosR - y * sinR;
    double ry = x * sinR + y * cosR;

    result->x = rx;
    result->y = ry;
}

void VROVector3d::rotateAboutAxis(const VROVector3d &axisDir, const VROVector3d &axisPos, double angleRad,
                                  VROVector3d *result) const {
    VROMatrix4d pivot;
    pivot.rotate(angleRad, axisPos, axisDir);
    pivot.multiplyVector(*this, result);
}

double VROVector3d::distanceXY(const VROVector3d &vector) const {
    return sqrt((vector.y - y) * (vector.y - y) + (vector.x - x) * (vector.x - x));
}

void VROVector3d::set(const VROVector3d &value) {
    x = value.x;
    y = value.y;
    z = value.z;
}

void VROVector3d::set(const VROVector3f &value) {
    x = value.x;
    y = value.y;
    z = value.z;
}

void VROVector3d::set(double x, double y, double z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

double VROVector3d::dot(const VROVector3d &vB) const {
    return x * vB.x + y * vB.y + z * vB.z;
}

double VROVector3d::distance(const VROVector3d &vB) const  {
    return sqrt((vB.z - z) * (vB.z - z) + (vB.y - y) * (vB.y - y) + (vB.x - x) * (vB.x - x));
}

double VROVector3d::distanceSquared(const VROVector3d &vB) const  {
    return (vB.z - z) * (vB.z - z) + (vB.y - y) * (vB.y - y) + (vB.x - x) * (vB.x - x);
}

void VROVector3d::add(const VROVector3d &vB, VROVector3d *result) const {
    result->x = x + vB.x;
    result->y = y + vB.y;
    result->z = z + vB.z;
}

void VROVector3d::addScaled(const VROVector3d &scaledB, float scale, VROVector3d *result) const {
    result->x = x + scaledB.x * scale;
    result->y = y + scaledB.y * scale;
    result->z = z + scaledB.z * scale;
}

void VROVector3d::subtract(const VROVector3d &vB, VROVector3d *result) const {
    result->x = x - vB.x;
    result->y = y - vB.y;
    result->z = z - vB.z;
}

void VROVector3d::midpoint(const VROVector3f &other, VROVector3d *result) const {
    result->x = (x + other.x) * 0.5f;
    result->y = (y + other.y) * 0.5f;
    result->z = (z + other.z) * 0.5f;
}

void VROVector3d::midpoint(const VROVector3d &other, VROVector3d *result) const {
    result->x = (x + other.x) * 0.5;
    result->y = (y + other.y) * 0.5;
    result->z = (z + other.z) * 0.5;
}

void VROVector3d::midpoint(const VROVector3f &other, VROVector3f *result) const {
    result->x = (x + other.x) * 0.5f;
    result->y = (y + other.y) * 0.5f;
    result->z = (z + other.z) * 0.5f;
}

void VROVector3d::midpoint(const VROVector3d &other, VROVector3f *result) const {
    result->x = (x + other.x) * 0.5;
    result->y = (y + other.y) * 0.5;
    result->z = (z + other.z) * 0.5;
}

void VROVector3d::cross(const VROVector3d &vB, VROVector3d *result) const {
    result->x = y * vB.z - z * vB.y;
    result->y = z * vB.x - x * vB.z;
    result->z = x * vB.y - y * vB.x;
}

double VROVector3d::normalize() {
    double magnitude = sqrt(x * x + y * y + z * z);
    if (magnitude == 0) {
        magnitude = 1; //prevent NaN
    }

    x = x / magnitude;
    y = y / magnitude;
    z = z / magnitude;

    return magnitude;
}

double VROVector3d::magnitude() const {
    return sqrt(x * x + y * y + z * z);
}

bool VROVector3d::isZero() const {
    return x == 0 && y == 0 && z == 0;
}

double VROVector3d::magnitudeXY() const {
    return sqrt(x * x + y * y);
}

void VROVector3d::scale(double factor, VROVector3d *result) const  {
    result->x = x * factor;
    result->y = y * factor;
    result->z = z * factor;
}

std::string VROVector3d::toString() const {
    std::stringstream ss;
    ss << "[x: " << x << ", y: " << y << ", z: " << z << "]";

    return ss.str();
}
