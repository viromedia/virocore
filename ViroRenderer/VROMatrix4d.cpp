//
//  VROMatrix4d.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMatrix4d.h"
#include "VROVector3d.h"
#include "VROMath.h"

VROMatrix4d::VROMatrix4d() {
    toIdentity();
}

VROMatrix4d::VROMatrix4d(const VROMatrix4d &toCopy) {
    memcpy(mtx, toCopy.mtx, sizeof(double) * 16);
}

VROMatrix4d::VROMatrix4d(const double *matrix) {
    memcpy(mtx, matrix, sizeof(double) * 16);
}

VROMatrix4d::~VROMatrix4d() {

}

void VROMatrix4d::toIdentity() {
    memset(mtx, 0, 16 * sizeof(double));
    mtx[0] = mtx[5] = mtx[10] = mtx[15] = 1;
}

void VROMatrix4d::copy(const VROMatrix4d &copy)  {
    memcpy(mtx, copy.mtx, sizeof(double) * 16);
}

void VROMatrix4d::rotateX(double angleRad) {
    double rsin = sin(angleRad);
    double rcos = cos(angleRad);

    for (int i = 0; i < 3; i++) {
        int i1 = i * 4 + 1;
        int i2 = i1 + 1;
        double t = mtx[i1];
        mtx[i1] = t * rcos - mtx[i2] * rsin;
        mtx[i2] = t * rsin + mtx[i2] * rcos;
    }
}

void VROMatrix4d::rotateY(double angleRad) {
    double rsin = sin(angleRad);
    double rcos = cos(angleRad);

    for (int i = 0; i < 3; i++) {
        int i0 = i * 4;
        int i2 = i0 + 2;
        double t = mtx[i0];
        mtx[i0] = t * rcos + mtx[i2] * rsin;
        mtx[i2] = mtx[i2] * rcos - t * rsin;
    }
}

void VROMatrix4d::rotateZ(double angleRad) {
    double rsin = sin(angleRad);
    double rcos = cos(angleRad);

    for (int i = 0; i < 3; i++) {
        int i0 = i * 4;
        int i1 = i0 + 1;
        double t = mtx[i0];
        mtx[i0] = t * rcos - mtx[i1] * rsin;
        mtx[i1] = t * rsin + mtx[i1] * rcos;
    }
}

void VROMatrix4d::rotate(double angleRad, const VROVector3d &origin, const VROVector3d &dir)  {
    double a = origin.x;
    double b = origin.y;
    double c = origin.z;

    double u = dir.x;
    double v = dir.y;
    double w = dir.z;
    double u2 = u * u;
    double v2 = v * v;
    double w2 = w * w;
    double l2 = u2 + v2 + w2;

    // Early out for short rotation vector
    if (l2 < 0.000000001) {
        return;
    }

    double cosT = cosf(angleRad);
    double sinT = sinf(angleRad);
    double l = sqrtf(l2);

    VROMatrix4d scratchMtx;
    double *txMtx = scratchMtx.mtx;

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

void VROMatrix4d::translate(double x, double y, double z) {
    VROMatrix4d translate;
    translate.toIdentity();

    translate.mtx[12] = x;
    translate.mtx[13] = y;
    translate.mtx[14] = z;

    postMultiply(translate);
}

void VROMatrix4d::translate(const VROVector3d &vector) {
    translate(vector.x, vector.y, vector.z);
}

void VROMatrix4d::scale(double x, double y, double z) {
    for (int i = 0; i < 3; i++) {
        int i0 = i * 4;
        mtx[i0] *= x;
        mtx[i0 + 1] *= y;
        mtx[i0 + 2] *= z;
    }
}

VROVector3d VROMatrix4d::multiply(const VROVector3d &vector) const  {
    VROVector3f result;
    result.x = vector.x * mtx[0] + vector.y * mtx[4] + vector.z * mtx[8] + mtx[12];
    result.y = vector.x * mtx[1] + vector.y * mtx[5] + vector.z * mtx[9] + mtx[13];
    result.z = vector.x * mtx[2] + vector.y * mtx[6] + vector.z * mtx[10] + mtx[14];
    
    return result;
}

VROMatrix4d VROMatrix4d::multiply(const VROMatrix4d &matrix)   {
    double nmtx[16];
    VROMathMultMatrices_d(matrix.mtx, mtx, nmtx);
    
    return VROMatrix4d(nmtx);
}
