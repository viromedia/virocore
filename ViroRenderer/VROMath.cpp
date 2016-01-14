//
//  VROMath.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROMath.h"
#include <algorithm>

VROMatrix4f matrix_from_scale(float sx, float sy, float sz) {
    VROMatrix4f m;
    m[0] = sx;
    m[5] = sy;
    m[10] = sz;
    
    return m;
}

VROMatrix4f matrix_from_translation(float x, float y, float z) {
    VROMatrix4f m;
    m[12] = x;
    m[13] = y;
    m[14] = z;
    
    return m;
}

VROMatrix4f matrix_from_rotation(float radians, float x, float y, float z) {
    vector_float3 v = vector_normalize(((vector_float3){x, y, z}));
    float cos = cosf(radians);
    float cosp = 1.0f - cos;
    float sin = sinf(radians);
    
    float m[16] = {
            cos + cosp * v.x * v.x,
            cosp * v.x * v.y + v.z * sin,
            cosp * v.x * v.z - v.y * sin,
            0.0f,
        
            cosp * v.x * v.y - v.z * sin,
            cos + cosp * v.y * v.y,
            cosp * v.y * v.z + v.x * sin,
            0.0f,
        
            cosp * v.x * v.z + v.y * sin,
            cosp * v.y * v.z - v.x * sin,
            cos + cosp * v.z * v.z,
            0.0f,
        
            0.0f, 0.0f, 0.0f, 1.0f
    };
    
    return VROMatrix4f(m);
}

VROMatrix4f matrix_from_perspective_fov_aspectLH(const float fovY, const float aspect, const float nearZ, const float farZ) {
    float yscale = 1.0f / tanf(fovY * 0.5f); // 1 / tan == cot
    float xscale = yscale / aspect;
    float q = farZ / (farZ - nearZ);
    
    float m[16] = {
        xscale, 0.0f, 0.0f, 0.0f,
        0.0f, yscale, 0.0f, 0.0f,
        0.0f, 0.0f, q, 1.0f,
        0.0f, 0.0f, q * -nearZ, 0.0f
    };
    
    return m;
}

VROMatrix4f matrix_for_frustum(const float left, const float right,
                                   const float bottom, const float top,
                                   const float znear, const float zfar) {
    const float x_2n = znear + znear;
    const float x_2nf = 2 * znear * zfar;
    
    const float p_fn = zfar + znear;
    const float m_nf = znear - zfar;
    
    const float p_rl = right + left;
    const float m_rl = right - left;
    const float p_tb = top + bottom;
    const float m_tb = top - bottom;
    
    float m[16] = {
           x_2n / m_rl, 0, 0, 0,
           0, x_2n / m_tb, 0, 0,
           p_rl / m_rl, p_tb / m_tb, p_fn / m_nf, -1,
           0, 0, x_2nf / m_nf, 0
    };

    return VROMatrix4f(m);
}

VROMatrix4f matrix_float4x4_from_GL(GLKMatrix4 glm) {
    float m[16] = {
        glm.m[0],  glm.m[1],  glm.m[2],  glm.m[3] ,
        glm.m[4],  glm.m[5],  glm.m[6],  glm.m[7] ,
        glm.m[8],  glm.m[9],  glm.m[10], glm.m[11],
        glm.m[12], glm.m[13], glm.m[14], glm.m[15]
    };
    
    return VROMatrix4f(m);
}

double degrees_to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

double radians_to_degrees(double radians) {
    return radians * 180.0 / M_PI;
}

float clamp(float val, float min, float max) {
    return std::max(min, std::min(max, val));
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Angle Computation
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Angle Computation

float toRadians(float degrees) {
    return degrees * M_PI / 180;
}

float toDegrees(float radians) {
    return radians * 180 / M_PI;
}

float VROMathNormalizeAngle2PI(float rad) {
    int numCirclesStart = rad / (2 * M_PI);
    rad = rad - numCirclesStart * (2 * M_PI);
    if (rad < 0) {
        rad += (2 * M_PI);
    }
    
    return rad;
}

float VROMathNormalizeAnglePI(float rad) {
    if (rad > -M_PI && rad < M_PI) {
        return rad;
    }
    
    float twopi = 2 * M_PI;
    
    float numCirclesStart = floor(rad / twopi);
    rad = rad - numCirclesStart * twopi;
    
    if (rad < -M_PI) {
        return twopi - rad;
    }
    else if (rad > M_PI) {
        return rad - twopi;
    }
    else {
        return rad;
    }
}

float VROMathAngleDistance(float radA, float radB) {
    radA = VROMathNormalizeAngle2PI(radA);
    radB = VROMathNormalizeAngle2PI(radB);
    
    /*
     Find the fastest direction from start to end: counter-clockwise
     or clockwise.
     */
    float radBNeg = radB - (2 * M_PI);
    float radBPos = radB + (2 * M_PI);
    
    if (fabs(radB - radA) > fabs(radBNeg - radA)) {
        radB = radBNeg;
    }
    if (fabs(radB - radA) > fabs(radBPos - radA)) {
        radB = radBPos;
    }
    
    /*
     Finally, compute the difference.
     */
    return fabs(radA - radB);
}

void VROMathRotateAroundZ(const VROVector3f &vector, float radians, VROVector3f *result) {
    float sinR = sin(radians);
    float cosR = cos(radians);
    
    result->x = vector.x * cosR - vector.y * sinR;
    result->y = vector.x * sinR + vector.y * cosR;
}

void VROMathRotateAroundX(const VROVector3f &vector, float radians, VROVector3f *result) {
    float sinR = sin(radians);
    float cosR = cos(radians);
    
    result->y = vector.y * cosR - vector.z * sinR;
    result->z = vector.y * sinR + vector.z * cosR;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Matrix Manipulation
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Matrix Manipulation

float IDENTITY_MATRIX[] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

double IDENTITY_MATRIX_D[] = { 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };

void VROMathMultVectorByMatrix(const float *matrix, const float *input, float *output) {
    for (int i = 0; i < 4; i++) {
        output[i] = input[0] * matrix[i] + input[1] * matrix[4 + i] + input[2] * matrix[8 + i] + input[3] * matrix[12 + i];
    }
}

void VROMathMultVectorByMatrix_d(const double *matrix, const double *input, double *output) {
    for (int i = 0; i < 4; i++) {
        output[i] = input[0] * matrix[i] + input[1] * matrix[4 + i] + input[2] * matrix[8 + i] + input[3] * matrix[12 + i];
    }
}

void VROMathMultVectorByMatrix_fd(const float *matrix, const double *input, double *output) {
    for (int i = 0; i < 4; i++) {
        output[i] = input[0] * matrix[i] + input[1] * matrix[4 + i] + input[2] * matrix[8 + i] + input[3] * matrix[12 + i];
    }
}

void VROMathMultMatrices(const float *m1, const float *m0, float *d) {
    d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
}

void VROMathMultMatrices_d(const double *m1, const double *m0, double *d) {
    d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
}

void VROMathMultMatrices_dff(const double *m1, const float *m0, float *d) {
    d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
}

void VROMathMultMatrices_ddf(const double *m1, const double *m0, float *d) {
    d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
}

void VROMathMultMatrices_fdf(const float *m1, const double *m0, float *d) {
    d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
}

void VROMathMultMatrices_dfd(const double *m1, const float *m0, double *d) {
    d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
}

void VROMathMultMatrices_fdd(const float *m1, const double *m0, double *d) {
    d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
}

void VROMathMultMatrices_ffd(const float *m1, const float *m0, double *d) {
    d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
    d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
    d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
    d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
    d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
    d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
    d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
    d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
    d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
    d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
    d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
    d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
}

void VROMathMultMatricesOptScale(const float *m1, const float *m0, float *d) {
    d[0] = m0[0] * m1[0];
    d[1] = m0[1] * m1[0];
    d[2] = m0[2] * m1[0];
    d[3] = m0[3] * m1[0];
    d[4] = m0[4] * m1[5];
    d[5] = m0[5] * m1[5];
    d[6] = m0[6] * m1[5];
    d[7] = m0[7] * m1[5];
    d[8] = m0[8];
    d[9] = m0[9];
    d[10] = m0[10];
    d[11] = m0[11];
    d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[12];
    d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[13];
    d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[14];
    d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[15];
}

void VROMathMultMVP(const float *m1, const float *m0, float *d) {
    d[0] = m0[0] * m1[0];
    d[1] = m0[5] * m1[1];
    d[2] = m0[10] * m1[2];
    d[3] = -m1[2];
    d[4] = m0[0] * m1[4];
    d[5] = m0[5] * m1[5];
    d[6] = m0[10] * m1[6];
    d[7] = -m1[6];
    d[8] = m0[0] * m1[8];
    d[9] = m0[5] * m1[9];
    d[10] = m0[10] * m1[10];
    d[11] = -m1[10];
    d[12] = m0[0] * m1[12];
    d[13] = m0[5] * m1[13];
    d[14] = m0[10] * m1[14] + m0[14];
    d[15] = -m1[14];
}

void VROMathMultVX(const float *vx, const float *m0, float *d) {
    /*
     Optimized multiply vx * mvp, knowing that:
     
     vx[1] = vx[2] = vx[3] = 0;
     vx[4] = vx[6] = vx[7] = 0;
     vx[8] = vx[9] = vx[11] = 0;
     vx[15] = 1
     */
    d[0] = m0[0] * vx[0];
    d[1] = m0[1] * vx[0];
    d[2] = m0[2] * vx[0];
    d[3] = m0[3] * vx[0];
    d[4] = m0[4] * vx[5];
    d[5] = m0[5] * vx[5];
    d[6] = m0[6] * vx[5];
    d[7] = m0[7] * vx[5];
    d[8] = m0[8] * vx[10];
    d[9] = m0[9] * vx[10];
    d[10] = m0[10] * vx[10];
    d[11] = m0[11] * vx[10];
    d[12] = m0[0] * vx[12] + m0[4] * vx[13] + m0[8] * vx[14] + m0[12];
    d[13] = m0[1] * vx[12] + m0[5] * vx[13] + m0[9] * vx[14] + m0[13];
    d[14] = m0[2] * vx[12] + m0[6] * vx[13] + m0[10] * vx[14] + m0[14];
    d[15] = m0[3] * vx[12] + m0[7] * vx[13] + m0[11] * vx[14] + m0[15];
}

void VROMathMakeIdentity(float *m) {
    memcpy(m, IDENTITY_MATRIX, sizeof(float) * 16);
}

void VROMathMakeIdentity_d(double *m) {
    memcpy(m, IDENTITY_MATRIX_D, sizeof(double) * 16);
}

void VROMathTransposeMatrix(const float *src, float *transpose) {
    transpose[0] = src[0];
    transpose[1] = src[4];
    transpose[2] = src[8];
    transpose[3] = src[12];
    
    transpose[4] = src[1];
    transpose[5] = src[5];
    transpose[6] = src[9];
    transpose[7] = src[13];

    transpose[8] = src[2];
    transpose[9] = src[6];
    transpose[10] = src[10];
    transpose[11] = src[14];

    transpose[12] = src[3];
    transpose[13] = src[7];
    transpose[14] = src[11];
    transpose[15] = src[15];
}

bool VROMathInvertMatrix(const float *src, float *inverse) {
    float temp[16];
    int i, j, k, swap;
    float t;
    
    for (int i = 0; i < 16; i++) {
        temp[i] = src[i];
    }
    
    VROMathMakeIdentity(inverse);
    
    for (i = 0; i < 4; i++) {
        /*
         * Look for largest element in column
         */
        swap = i;
        for (j = i + 1; j < 4; j++) {
            if (std::abs(temp[(j << 2) + i]) > std::abs(temp[(i << 2) + i])) {
                swap = j;
            }
        }
        
        if (swap != i) {
            /*
             * Swap rows.
             */
            for (k = 0; k < 4; k++) {
                t = temp[(i << 2) + k];
                temp[(i << 2) + k] = temp[(swap << 2) + k];
                temp[(swap << 2) + k] = t;
                
                t = inverse[(i << 2) + k];
                inverse[(i << 2) + k] = inverse[(swap << 2) + k];
                
                inverse[(swap << 2) + k] = t;
            }
        }
        
        if (temp[(i << 2) + i] == 0) {
            /*
             * No non-zero pivot. The matrix is singular, which shouldn't
             * happen. This means the user gave us a bad matrix.
             */
            return false;
        }
        
        t = temp[(i << 2) + i];
        for (k = 0; k < 4; k++) {
            temp[(i << 2) + k] /= t;
            inverse[(i << 2) + k] = inverse[(i << 2) + k] / t;
        }
        for (j = 0; j < 4; j++) {
            if (j != i) {
                t = temp[(j << 2) + i];
                for (k = 0; k < 4; k++) {
                    temp[(j << 2) + k] -= temp[(i << 2) + k] * t;
                    inverse[(j << 2) + k] = inverse[(j << 2) + k] - inverse[(i << 2) + k] * t;
                }
            }
        }
    }
    return true;
}

bool VROMathInvertMatrix_d(const double *src, double *inverse) {
    double temp[16];
    int i, j, k, swap;
    double t;
    
    for (int i = 0; i < 16; i++) {
        temp[i] = src[i];
    }
    
    VROMathMakeIdentity_d(inverse);
    
    for (i = 0; i < 4; i++) {
        /*
         * Look for largest element in column
         */
        swap = i;
        for (j = i + 1; j < 4; j++) {
            if (std::abs(temp[(j << 2) + i]) > std::abs(temp[(i << 2) + i])) {
                swap = j;
            }
        }
        
        if (swap != i) {
            /*
             * Swap rows.
             */
            for (k = 0; k < 4; k++) {
                t = temp[(i << 2) + k];
                temp[(i << 2) + k] = temp[(swap << 2) + k];
                temp[(swap << 2) + k] = t;
                
                t = inverse[(i << 2) + k];
                inverse[(i << 2) + k] = inverse[(swap << 2) + k];
                
                inverse[(swap << 2) + k] = t;
            }
        }
        
        if (temp[(i << 2) + i] == 0) {
            /*
             * No non-zero pivot. The matrix is singular, which shouldn't
             * happen. This means the user gave us a bad matrix.
             */
            return false;
        }
        
        t = temp[(i << 2) + i];
        for (k = 0; k < 4; k++) {
            temp[(i << 2) + k] /= t;
            inverse[(i << 2) + k] = inverse[(i << 2) + k] / t;
        }
        for (j = 0; j < 4; j++) {
            if (j != i) {
                t = temp[(j << 2) + i];
                for (k = 0; k < 4; k++) {
                    temp[(j << 2) + k] -= temp[(i << 2) + k] * t;
                    inverse[(j << 2) + k] = inverse[(j << 2) + k] - inverse[(i << 2) + k] * t;
                }
            }
        }
    }
    
    return true;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Interpolation
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Interpolation

float VROMathInterpolate(float input, float inMin, float inMax, float outMin, float outMax) {
    if (input < inMin) {
        return outMin;
    }
    if (input > inMax) {
        return outMax;
    }
    
    float outRange = outMax - outMin;
    float inRange = inMax - inMin;
    
    float position = (input - inMin) / inRange * outRange;
    return outMin + position;
}

double VROMathInterpolate_d(double input, double inMin, double inMax, double outMin, double outMax) {
    if (input < inMin) {
        return outMin;
    }
    if (input > inMax) {
        return outMax;
    }
    
    double outRange = outMax - outMin;
    double inRange = inMax - inMin;
    
    double position = (input - inMin) / inRange * outRange;
    return outMin + position;
}

float VROMathInterpolateMultistage(float input, int numStages, const float *inputs, float *outputs) {
    if (input < inputs[0]) {
        return outputs[0];
    }
    if (input >= inputs[numStages - 1]) {
        return outputs[numStages - 1];
    }
    
    float output = 1;
    for (int i = 1; i < numStages; i++) {
        if (input < inputs[i]) {
            output = VROMathInterpolate(input, inputs[i - 1], inputs[i], outputs[i - 1], outputs[i]);
            break;
        }
    }
    
    return output;
}

void VROMathInterpolatePoint(const float *bottom, const float *top, float amount, int size, float *result) {
    for (int i = 0; i < size; i++) {
        result[i] = bottom[i] + amount * (top[i] - bottom[i]);
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Clamping
//
/////////////////////////////////////////////////////////////////////////////////

double VROMathClamp(double input, double min, double max) {
    if (input < min) {
        return min;
    } else if (input > max) {
        return max;
    } else {
        return input;
    }
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Array math
//
/////////////////////////////////////////////////////////////////////////////////

float VROMathMin(const float values[], int count) {
    if (count < 1) {
        return FLT_MAX;
    }
    float minValue = values[0];
    for (int i = 1; i < count; i++) {
        minValue = std::min(minValue, values[i]);
    }
    return minValue;
}

float VROMathMax(const float values[], int count) {
    if (count < 1) {
        return FLT_MIN;
    }
    float maxValue = values[0];
    for (int i = 1; i < count; i++) {
        maxValue = std::max(maxValue, values[i]);
    }
    return maxValue;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Square Root
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Square Root

/*
 * The following gives sqrt(x) approximation that has 2.98 digits of
 * precision for the range [0.25, 1.0]. Values that fall outside that
 * range are automatically range reduced/increased in a way that doesn't
 * loose accuracy.
 *
 * The polynomial coefficients came from Computer Approximations, John
 * Fraser Hart, 1978. The selected polynomial is index 0072 on page
 * 94, sqrt(x) ~= P(x) for x in [0.25, 1.0], the coefficients are
 * found on page 156.
 *
 * The following paper helps explain how to use the book, which was
 * written in a very mathematically rigorous fashion:
 *
 *    http://www.ganssle.com/approx/approx-2.pdf
 */

float
VROMathFastSquareRoot(float x) {
    const float p00 =  0.217018672;
    const float p01 =  1.32256386;
    const float p02 = -0.825888891;
    const float p03 =  0.287369824;
    
    if (x == 0.0) {
        return x;
    }
    
    /*
     * To range reduce to [0.25, 1.0], we take advantage of the equality:
     *
     *    sqrt(x) = 2^k * sqrt(x * 2^(-2*k))
     *
     * As such we loop until we find a 'k' that reduces 'x' into the range we
     * need, and then adjust the final calculation by 2^k.
     */
    int k = 0;
    while (x < 0.25) {
        x *= 4.0;
        k--;
    }
    while (x > 1.0) {
        x /= 4.0;
        k++;
    }
    
    const float x_2 = x * x;
    const float x_3 = x_2 * x;
    
    if(k >= 0) {
        return (p00 + p01 * x + p02 * x_2 + p03 * x_3) * (1 << k);
    }
    else {
        return (p00 + p01 * x + p02 * x_2 + p03 * x_3) / (1 << -k);
    }
}

/*
 * The following gives a sincos() approximation that has 5.17 digits
 * of precision. The algorithm a second order polynomial to
 * approximate cos() for an interval of [0, pi/2]. Since sin() lags
 * cos(), we can share most of the initial range-reduction
 * calculations and use the same polynomial approximation to get both.
 *
 * The polynomial coefficients came from Computer Approximations, John
 * Fraser Hart, 1978. The selected polynomial is index 3501 on page 118,
 * cos(x) ~= P(x^2), the coefficients are found on page 207.
 *
 * The following paper helps explain how to use the book, which was
 * written in a very mathematically rigorous fashion:
 *
 *    http://www.ganssle.com/approx/approx-2.pdf
 */
void
VROMathFastSinCos(float x, float r[2]) {
    const float p0 =  0.9999932946;
    const float p1 = -0.4999124376;
    const float p2 =  0.0414877472;
    const float p3 = -0.00127120948;
    const float two_pi = M_PI * 2;
    
    // reduce range to (-2*pi, 2*pi)
    const float x_abs = fabsf(x);
    if (x_abs > two_pi) {
        const int shifts = x_abs / two_pi;
        x += (x < 0 ? shifts : -shifts) * two_pi;
    }
    
    // cos(x) = cos(-x), sin(-x) = -sin(x); this gives us [0, 2*pi)
    bool orig_neg = x < 0;
    x = fabsf(x);
    
    // by flipping vertically and horizontally, the shape of [0, pi/2] can be used to define the
    // four phases of a sinusoidal. here we figure out which phase we're in, do the folding to
    // reduce to [0, pi/2), and setup the sign to compensate for the flipping. Since sin() lags
    // cos() [eg sin(x) = cos(x - pi/2)], we can also setup sin().
    float x_cos;
    float x_sin;
    bool cos_neg;
    bool sin_neg;
    if (x < M_PI) {
        if (x < M_PI_2) {
            cos_neg = false;
            sin_neg = orig_neg;
            x_cos = x;
            x_sin = M_PI_2 - x;
        } else  {
            cos_neg = true;
            sin_neg = orig_neg;
            x_cos = M_PI - x;
            x_sin = x - M_PI_2;
        }
    } else {
        if (x < (M_PI_2 + M_PI)) {
            cos_neg = true;
            sin_neg = ! orig_neg;
            x_cos = x - M_PI;
            x_sin = (M_PI + M_PI_2) - x;
        } else {
            cos_neg = false;
            sin_neg = ! orig_neg;
            x_cos = two_pi - x;
            x_sin = x - (M_PI + M_PI_2);
        }
    }
    
    const float x_cos_2 = x_cos * x_cos;
    const float x_cos_4 = x_cos_2 * x_cos_2;
    const float x_cos_6 = x_cos_4 * x_cos_2;
    const float x_sin_2 = x_sin * x_sin;
    const float x_sin_4 = x_sin_2 * x_sin_2;
    const float x_sin_6 = x_sin_4 * x_sin_2;
    
    const double r_cos_abs = p0 + p1 * x_cos_2 + p2 * x_cos_4 + p3 * x_cos_6;
    const double r_sin_abs = p0 + p1 * x_sin_2 + p2 * x_sin_4 + p3 * x_sin_6;
    r[1] = cos_neg ? -r_cos_abs : r_cos_abs;
    r[0] = sin_neg ? -r_sin_abs : r_sin_abs;
}

//
// TESTING CODE: this was helpful for verifying that VROMathFastSinCos() was working right
//
//    gcc -Wall -std=gnu99 -lm -o VROMath VROMath.cpp
//
// int
// main(int argc, char **argv) {
//     for (int i = -62832; i < 62832; i++) {
//         float x = (double)i * 0.001;
//         float rs[2];
//         VROMathFastSinCos(x, rs);
//         float actualsin = sinf(x);
//         float actualcos = cosf(x);
//         if (fabsf(rs[0] - actualsin) > 0.001f) {
//             printf("sin %7.3f: %14.7f %14.7f %14.7f\n", x, rs[0], actualsin, rs[0] - actualsin);
//         }
//         if (fabsf(rs[1] - actualcos) > 0.001f) {
//             printf("cos %7.3f: %14.7f %14.7f %14.7f\n", x, rs[1], actualcos, rs[1] - actualcos);
//         }
//     }
//     return 0;
// }
//
// END TESTING CODE
//

void VROMathFastSinCos2x(const float *angles, float *r) {
    VROMathFastSinCos(angles[0], r);
    VROMathFastSinCos(angles[1], r + 2);
}

float VROMathReciprocal(float value) {
    return 1.0f / value;
}

float VROMathReciprocalSquareRoot(float value) {
    return 1.0f / sqrt(value);
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Miscellaneous
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Miscellaneous

bool VROMathIsZero(const float a, const float tolerance) {
    return fabs(a) <= tolerance;
}

bool VROMathEquals(const float a, const float b, const float tolerance) {
    return (a + tolerance >= b) && (a - tolerance <= b);
}

float VROFloat16ToFloat(short fltInt16) {
    int fltInt32    =  ((fltInt16 & 0x8000) << 16);
    fltInt32        |= ((fltInt16 & 0x7fff) << 13) + 0x38000000;
    
    float fRet;
    memcpy(&fRet, &fltInt32, sizeof(float));
    return fRet;
}

/////////////////////////////////////////////////////////////////////////////////
//
//  Geometry
//
/////////////////////////////////////////////////////////////////////////////////
#pragma mark -
#pragma mark Geometry

/**
 Compare a point (x,y) and a line (x1,y1 - x2, y2)
 Returns 0 if on line. Returns positive if on one side, negative if other.
 
 See http://mathforum.org/library/drmath/view/54386.html for details.
 */
inline float comparePointLine(float x, float y, float x1, float y1, float x2, float y2) {
    return y - y1 - (((y2 - y1)/(x2- x1)) * (x - x1));
}

/**
 Determine if point (x,y) is 'inside' or on a line(x1,y1 - x2, y2).
 Inside is determine by another point on the polygon (xOther, yOther).
 Polygon must be convex.
 */
inline bool pointIsInsideLine(float x, float y, float x1, float y1, float x2, float y2, float xOther, float yOther) {
    bool insideNegative = (comparePointLine(xOther, yOther, x1, y1, x2, y2) < 0);
    float point = comparePointLine(x, y, x1, y1, x2, y2);
    return (point == 0 || (point < 0) == insideNegative);
}

bool VROMathPointIsInPolygon(float x, float y, float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4) {
    if (pointIsInsideLine(x, y, x1, y1, x2, y2, x3, y3)) {
        return true;
    }
    if (pointIsInsideLine(x, y, x2, y2, x3, y3, x4, y4)) {
        return true;
    }
    if (pointIsInsideLine(x, y, x3, y3, x4, y4, x1, y1)) {
        return true;
    }
    if (pointIsInsideLine(x, y, x4, y4, x1, y1, x2, y2)) {
        return true;
    }
    return false;
}

void VROMathGetClosestPointOnSegment(const VROVector3d &A, const VROVector3d &B, const VROVector3d &p, VROVector3d &result) {
    if (p.isEqual(A)) {
        result.set(A);
    }
    else if (p.isEqual(B)) {
        result.set(B);
    }
    else {
        double dx = B.x - A.x;
        double dy = B.y - A.y;
        double dz = B.z - A.z;
        double lengthSq = dx * dx + dy * dy + dz * dz;
        
        double t = ((p.x - A.x) * dx + (p.y - A.y) * dy + (p.z - A.z) * dz) / lengthSq;
        
        /*
         * t gives us the projection factor that determines the point on the line that's closest to p. If t
         * is less than 0 or greater than 1, though, then we're off the segment, so we clamp at 0 and 1.
         */
        if (t < 0.0) {
            t = 0.0;
        }
        else if (t > 1.0) {
            t = 1.0;
        }
        
        result.x = A.x + dx * t;
        result.y = A.y + dy * t;
        result.z = A.z + dz * t;
    }
}

