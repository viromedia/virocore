//
//  VROMatrix4f.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMatrix4f.h"
#include "VROMath.h"
#include "VROLog.h"
#include <sstream>

VROMatrix4f::VROMatrix4f() {
    toIdentity();
}

VROMatrix4f::VROMatrix4f(const float *matrix) {
    memcpy(_mtx, matrix, sizeof(float) * 16);
}

VROMatrix4f::~VROMatrix4f() {

}

void VROMatrix4f::toIdentity() {
    memset(_mtx, 0, 16 * sizeof(float));
    _mtx[0] = _mtx[5] = _mtx[10] = _mtx[15] = 1;
}

void VROMatrix4f::copy(const VROMatrix4f &copy)  {
    memcpy(_mtx, copy._mtx, sizeof(float) * 16);
}

void VROMatrix4f::rotateX(float angleRad) {
    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    float rsin = sincosr[0];
    float rcos = sincosr[1];

    for (int i = 0; i < 3; i++) {
        int i1 = i * 4 + 1;
        int i2 = i1 + 1;
        float t = _mtx[i1];
        _mtx[i1] = t * rcos - _mtx[i2] * rsin;
        _mtx[i2] = t * rsin + _mtx[i2] * rcos;
    }
}

void VROMatrix4f::rotateY(float angleRad) {
    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    float rsin = sincosr[0];
    float rcos = sincosr[1];

    for (int i = 0; i < 3; i++) {
        int i0 = i * 4;
        int i2 = i0 + 2;
        float t = _mtx[i0];
        _mtx[i0] = t * rcos + _mtx[i2] * rsin;
        _mtx[i2] = _mtx[i2] * rcos - t * rsin;
    }
}

void VROMatrix4f::rotateZ(float angleRad) {
    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    float rsin = sincosr[0];
    float rcos = sincosr[1];

    for (int i = 0; i < 3; i++) {
        int i0 = i * 4;
        int i1 = i0 + 1;
        float t = _mtx[i0];
        _mtx[i0] = t * rcos - _mtx[i1] * rsin;
        _mtx[i1] = t * rsin + _mtx[i1] * rcos;
    }
}

void VROMatrix4f::rotate(float angleRad, const VROVector3f &origin, const VROVector3f &dir) {
    float a = origin.x;
    float b = origin.y;
    float c = origin.z;

    float u = dir.x;
    float v = dir.y;
    float w = dir.z;
    float u2 = u * u;
    float v2 = v * v;
    float w2 = w * w;
    float l2 = u2 + v2 + w2;

    // Early out for short rotation vector
    if (l2 < 0.000000001f) {
        return;
    }

    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    float sinT = sincosr[0];
    float cosT = sincosr[1];
    float l = VROMathFastSquareRoot(l2);

    VROMatrix4f txMtx;
    txMtx[0] = (u2 + (v2 + w2) * cosT) / l2;
    txMtx[1] = (u * v * (1 - cosT) + w * l * sinT) / l2;
    txMtx[2] = (u * w * (1 - cosT) - v * l * sinT) / l2;
    txMtx[3] = 0;

    txMtx[4] = (u * v * (1 - cosT) - w * l * sinT) / l2;
    txMtx[5] = (v2 + (u2 + w2) * cosT) / l2;
    txMtx[6] = (v * w * (1 - cosT) + u * l * sinT) / l2;
    txMtx[7] = 0;

    txMtx[8] = (u * w * (1 - cosT) + v * l * sinT) / l2;
    txMtx[9] = (v * w * (1 - cosT) - u * l * sinT) / l2;
    txMtx[10] = (w2 + (u2 + v2) * cosT) / l2;
    txMtx[11] = 0;

    txMtx[12] = ((a * (v2 + w2) - u * (b * v + c * w)) * (1 - cosT) + (b * w - c * v) * l * sinT) / l2;
    txMtx[13] = ((b * (u2 + w2) - v * (a * u + c * w)) * (1 - cosT) + (c * u - a * w) * l * sinT) / l2;
    txMtx[14] = ((c * (u2 + v2) - w * (a * u + b * v)) * (1 - cosT) + (a * v - b * u) * l * sinT) / l2;
    txMtx[15] = 1;

    VROMatrix4f result = multiply(txMtx);
    memcpy(_mtx, result._mtx, sizeof(float) * 16);
}

void VROMatrix4f::translate(float x, float y, float z) {
    _mtx[12] += x;
    _mtx[13] += y;
    _mtx[14] += z;
}

void VROMatrix4f::translate(const VROVector3f &vector) {
    translate(vector.x, vector.y, vector.z);
}

void VROMatrix4f::scale(float x, float y, float z) {
    for (int i = 0; i < 3; i++) {
        int i0 = i * 4;
        _mtx[i0] *= x;
        _mtx[i0 + 1] *= y;
        _mtx[i0 + 2] *= z;
    }
}

VROVector3f VROMatrix4f::multiply(const VROVector3f &vector) const {
    VROVector3f result;
    result.x = vector.x * _mtx[0] + vector.y * _mtx[4] + vector.z * _mtx[8] + _mtx[12];
    result.y = vector.x * _mtx[1] + vector.y * _mtx[5] + vector.z * _mtx[9] + _mtx[13];
    result.z = vector.x * _mtx[2] + vector.y * _mtx[6] + vector.z * _mtx[10] + _mtx[14];
    
    return result;
}

VROMatrix4f VROMatrix4f::multiply(const VROMatrix4f &matrix) const {
    float nmtx[16];
    VROMathMultMatrices(matrix._mtx, _mtx, nmtx);

    return VROMatrix4f(nmtx);
}

void VROMatrix4f::setRotationCenter(const VROVector3f &center, const VROVector3f &translation) {
    _mtx[12] = -_mtx[0] * center.x - _mtx[4] * center.y - _mtx[8]  * center.z + (center.x - translation.x);
    _mtx[13] = -_mtx[1] * center.x - _mtx[5] * center.y - _mtx[9]  * center.z + (center.y - translation.y);
    _mtx[14] = -_mtx[2] * center.x - _mtx[6] * center.y - _mtx[10] * center.z + (center.z - translation.z);
    _mtx[15] = 1.0;
}

VROMatrix4f VROMatrix4f::transpose() const {
    float transpose[16];
    VROMathTransposeMatrix(_mtx, transpose);
    
    return VROMatrix4f(transpose);
}

VROMatrix4f VROMatrix4f::invert() const {
    float inverted[16];
    VROMathInvertMatrix(_mtx, inverted);
    
    return VROMatrix4f(inverted);
}

std::string VROMatrix4f::toString() const {
    std::ostringstream ss;
    ss << _mtx[0] << ", " << _mtx[4] << ", " << _mtx[8] << ", " << _mtx[12] << "\n";
    ss << _mtx[1] << ", " << _mtx[5] << ", " << _mtx[9] << ", " << _mtx[13] << "\n";
    ss << _mtx[2] << ", " << _mtx[6] << ", " << _mtx[10] << ", " << _mtx[14] << "\n";
    ss << _mtx[3] << ", " << _mtx[7] << ", " << _mtx[11] << ", " << _mtx[15];
    
    return ss.str();
}

