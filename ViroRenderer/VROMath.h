//
//  VROMath.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMath_h
#define VROMath_h

#include <stdio.h>
#include <simd/simd.h>
#include <GLKit/GLKit.h>
#include "VROVector3f.h"
#include "VROVector3d.h"
#include "VROMatrix4f.h"
#include "VROMatrix4d.h"

static float kRoundingErrorFloat = 0.00001;

matrix_float4x4 matrix_from_scale(float sx, float sy, float sz);
matrix_float4x4 matrix_from_translation(float x, float y, float z);
matrix_float4x4 matrix_from_rotation(float radians, float x, float y, float z);
matrix_float4x4 matrix_from_perspective_fov_aspectLH(const float fovY, const float aspect,
                                                     const float nearZ, const float farZ);
matrix_float4x4 matrix_for_frustum(const float left, const float right,
                                   const float bottom, const float top,
                                   const float znear, const float zfar);

matrix_float4x4 matrix_float4x4_from_GL(GLKMatrix4 glmatrix);

double degrees_to_radians(double degrees);
double radians_to_degrees(double radians);

float clamp(float val, float min, float max);

/*
 4x4 column-major matrix operations.
 */
void VROMathMultVectorByMatrix(const float *matrix, const float *input, float *output);
void VROMathMultVectorByMatrix_d(const double *matrix, const double *input, double *output);
void VROMathMultVectorByMatrix_fd(const float *matrix, const double *input, double *output);

void VROMathMultMatrices(const float *a, const float *b, float *r);
void VROMathMultMatrices_d(const double *a, const double *b, double *r);
void VROMathMultMatrices_dff(const double *a, const float *b, float *r);
void VROMathMultMatrices_ddf(const double *a, const double *b, float *r);
void VROMathMultMatrices_fdf(const float *a, const double *b, float *r);
void VROMathMultMatrices_dfd(const double *a, const float *b, double *r);
void VROMathMultMatrices_fdd(const float *a, const double *b, double *r);
void VROMathMultMatrices_ffd(const float *a, const float *b, double *r);

void VROMathMakeIdentity(float *m);
void VROMathMakeIdentity_d(double *m);

bool VROMathInvertMatrix(const float *src, float *inverse);
bool VROMathInvertMatrix_d(const double *src, double *inverse);

/*
 4x4 special matrix ops.
 */
void VROMathMultMatricesOptScale(const float *m1, const float *m0, float *d);
void VROMathMultMVP(const float *m1, const float *m0, float *d);
void VROMathMultVX(const float *vx, const float *m0, float *d);

/*
 Interpolation functions.
 */
float  VROMathInterpolate(float input, float inMin, float inMax, float outMin, float outMax);
double VROMathInterpolate_d(double input, double inMin, double inMax, double outMin, double outMax);
float  VROMathInterpolateMultistage(float input, int numStages, const float *inputs, float *outputs);
void   VROMathInterpolatePoint(const float *bottom, const float *top, float amount, int size, float *result);

/*
 * Clamps input between min and max
 */
double VROMathClamp(double input, double min, double max);

/*
 Array math
 */
float VROMathMin(const float values[], int count);
float VROMathMax(const float values[], int count);

/*
 Angle conversion.
 */
float toRadians(float degrees);
float toDegrees(float radians);

/*
 Rotation.
 */
void VROMathRotateAroundX(const VROVector3f &vector, float radians, VROVector3f *result);
void VROMathRotateAroundZ(const VROVector3f &vector, float radians, VROVector3f *result);

/*
 Normalize the given angle between [0,2PI] or [-PI,PI], and find the distance between two angles.
 */
float VROMathNormalizeAngle2PI(float rad);
float VROMathNormalizeAnglePI(float rad);
float VROMathAngleDistance(float radA, float radB);

/*
 Take the fast (inverse) square root of the given number.
 */
float VROMathFastSquareRoot(float x);

/*
 Fast sin/cos methods. An input angle between [-PI, PI] will skip a
 range reduction step, and may perform slightly faster, but is
 unnessisary.
 */
void  VROMathFastSinCos(float x, float r[2]);

void  VROMathFastSinCos2x(const float *angles, float * r);

/**
 Determine whether point (x,y) is within polygon (x1,y1 to x2,y2 to x3,y3 to x4,y4 to x1,y1)
 Point on edge is considered within.
 Only for use with convex polygons.
 */
bool VROMathPointIsInPolygon(float x, float y, float x1, float y1,
                             float x2, float y2, float x3, float y3,
                             float x4, float y4);

/*
 Get the point on segment AB that is closest to p.
 */
void VROMathGetClosestPointOnSegment(const VROVector3d &A, const VROVector3d &B, const VROVector3d &p, VROVector3d &result);

/* return the power of 2 that is equal or greater to the given value */
static inline uint32_t
power2_ceil(const uint32_t v) {
    return  (v < 2) ? v + 1 : 1 << (sizeof(uint32_t) * 8 - __builtin_clz(v - 1));
}

float VROMathReciprocal(float value) {
    return 1.0f / value;
}

float VROMathReciprocalSquareRoot(float value) {
    return 1.0f / sqrt(value);
}

bool VROMathIsZero(const float a, const float tolerance = kRoundingErrorFloat) {
    return fabs(a) <= tolerance;
}

bool VROMathEquals(const float a, const float b, const float tolerance = kRoundingErrorFloat) {
    return (a + tolerance >= b) && (a - tolerance <= b);
}

#endif /* VROMath_h */
