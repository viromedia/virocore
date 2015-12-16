//
//  VROVector4f.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROVector4f.h"
#include <sstream>

VROVector4f::VROVector4f() {
    x = 0;
    y = 0;
    z = 0;
}

VROVector4f::VROVector4f(float xIn, float yIn, float zIn, float wIn) :
    x(xIn), y(yIn), z(zIn), w(wIn) {
    
}

VROVector4f::VROVector4f(const VROVector4f &vector) :
    x(vector.x),
    y(vector.y),
    z(vector.z),
    w(vector.w)
{}

VROVector4f::~VROVector4f() {
    
}

int VROVector4f::hash() const {
    return floor(x + 31 * y + 31 * z + 121 * w);
}

bool VROVector4f::isEqual(const VROVector4f &vertex) const {
    return fabs(x - vertex.x) < .00001 &&
           fabs(y - vertex.y) < .00001 &&
           fabs(z - vertex.z) < .00001 &&
           fabs(w - vertex.w) < .00001;
}

void VROVector4f::clear() {
    x = 0;
    y = 0;
    z = 0;
    w = 0;
}

void VROVector4f::set(const VROVector4f &value) {
    x = value.x;
    y = value.y;
    z = value.z;
    w = value.w;
}

void VROVector4f::set(float x, float y, float z, float w) {
    this->x = x;
    this->y = y;
    this->z = z;
    this->w = w;
}

float VROVector4f::dot(const VROVector4f &vB) const {
    return x * vB.x + y * vB.y + z * vB.z + w * vB.w;
}

void VROVector4f::add(const VROVector4f &vB, VROVector4f *result) const {
    result->x = x + vB.x;
    result->y = y + vB.y;
    result->z = z + vB.z;
    result->w = w + vB.w;
}

void VROVector4f::addScaled(const VROVector4f &scaledB, float scale, VROVector4f *result) const {
    result->x = x + scaledB.x * scale;
    result->y = y + scaledB.y * scale;
    result->z = z + scaledB.z * scale;
    result->w = w + scaledB.w * scale;
}

void VROVector4f::subtract(const VROVector4f &vB, VROVector4f *result) const {
    result->x = x - vB.x;
    result->y = y - vB.y;
    result->z = z - vB.z;
    result->w = w - vB.w;
}

void VROVector4f::midpoint(const VROVector4f &other, VROVector4f *result) const {
    result->x = (x + other.x) * 0.5f;
    result->y = (y + other.y) * 0.5f;
    result->z = (z + other.z) * 0.5f;
    result->w = (w + other.w) * 0.5f;
}

void VROVector4f::normalize() {
    float inverseMag = 1.0f / sqrtf(x * x + y * y + z * z + w * w);
    
    this->x *= inverseMag;
    this->y *= inverseMag;
    this->z *= inverseMag;
    this->w *= inverseMag;
}

float VROVector4f::magnitude() const {
    return sqrt(x * x + y * y + z * z + w * w);
}

bool VROVector4f::isZero() const {
    return x == 0 && y == 0 && z == 0 && w == 0;
}

void VROVector4f::scale(float factor, VROVector4f *result) const {
    result->x = x * factor;
    result->y = y * factor;
    result->z = z * factor;
    result->w = w * factor;
}

VROVector4f VROVector4f::interpolate(VROVector4f other, float t) {
    VROVector4f result;
    result.x = x + (other.x - x) * t;
    result.y = y + (other.y - y) * t;
    result.z = z + (other.z - z) * t;
    result.w = w + (other.w - w) * t;
    
    return result;
}

std::string VROVector4f::toString() const {
    std::stringstream ss;
    ss << "[x: " << x << ", y: " << y << ", z: " << z << "]";
    
    return ss.str();
}