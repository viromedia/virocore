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

VROMatrix4f::VROMatrix4f() {
    toIdentity();
}

VROMatrix4f::VROMatrix4f(const float *matrix) {
    memcpy(mtx, matrix, sizeof(float) * 16);
}

VROMatrix4f::~VROMatrix4f() {

}

void VROMatrix4f::toIdentity() {
    memset(mtx, 0, 16 * sizeof(float));
    mtx[0] = mtx[5] = mtx[10] = mtx[15] = 1;
}

void VROMatrix4f::copy(const VROMatrix4f &copy)  {
    memcpy(mtx, copy.mtx, sizeof(float) * 16);
}

void VROMatrix4f::rotateX(float angleRad) {
    float sincosr[2];
    VROMathFastSinCos(VROMathNormalizeAnglePI(angleRad), sincosr);

    float rsin = sincosr[0];
    float rcos = sincosr[1];

    for (int i = 0; i < 3; i++) {
        int i1 = i * 4 + 1;
        int i2 = i1 + 1;
        float t = mtx[i1];
        mtx[i1] = t * rcos - mtx[i2] * rsin;
        mtx[i2] = t * rsin + mtx[i2] * rcos;
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
        float t = mtx[i0];
        mtx[i0] = t * rcos + mtx[i2] * rsin;
        mtx[i2] = mtx[i2] * rcos - t * rsin;
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
        float t = mtx[i0];
        mtx[i0] = t * rcos - mtx[i1] * rsin;
        mtx[i1] = t * rsin + mtx[i1] * rcos;
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

    VROMatrix4f scratchMtx;
    float *txMtx = scratchMtx.mtx;

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

    postMultiply(scratchMtx);
}

void VROMatrix4f::translate(float x, float y, float z) {
    mtx[12] += x;
    mtx[13] += y;
    mtx[14] += z;
}

void VROMatrix4f::translate(const VROVector3f &vector) {
    translate(vector.x, vector.y, vector.z);
}

void VROMatrix4f::scale(float x, float y, float z) {
    for (int i = 0; i < 3; i++) {
        int i0 = i * 4;
        mtx[i0] *= x;
        mtx[i0 + 1] *= y;
        mtx[i0 + 2] *= z;
    }
}

void VROMatrix4f::multiplyVector(const VROVector3f &vector, VROVector3f *result) const  {
    result->x = vector.x * mtx[0] + vector.y * mtx[4] + vector.z * mtx[8] + mtx[12];
    result->y = vector.x * mtx[1] + vector.y * mtx[5] + vector.z * mtx[9] + mtx[13];
    result->z = vector.x * mtx[2] + vector.y * mtx[6] + vector.z * mtx[10] + mtx[14];
}

void VROMatrix4f::preMultiply(const VROMatrix4f &AM)   {
    float nmtx[16];
    VROMathMultMatrices(AM.mtx, mtx, nmtx);
    memcpy(mtx, nmtx, sizeof(float) * 16);
}

void VROMatrix4f::postMultiply(const VROMatrix4f &BM)  {
    float nmtx[16];
    VROMathMultMatrices(mtx, BM.mtx, nmtx);
    memcpy(mtx, nmtx, sizeof(float) * 16);
}

void VROMatrix4f::setRotationCenter(const VROVector3f &center, const VROVector3f &translation) {
    mtx[12] = -mtx[0] * center.x - mtx[4] * center.y - mtx[8]  * center.z + (center.x - translation.x);
    mtx[13] = -mtx[1] * center.x - mtx[5] * center.y - mtx[9]  * center.z + (center.y - translation.y);
    mtx[14] = -mtx[2] * center.x - mtx[6] * center.y - mtx[10] * center.z + (center.z - translation.z);
    mtx[15] = 1.0;
}
