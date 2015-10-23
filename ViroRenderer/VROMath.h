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

matrix_float4x4 matrix_from_scale(float sx, float sy, float sz);
matrix_float4x4 matrix_from_translation(float x, float y, float z);
matrix_float4x4 matrix_from_rotation(float radians, float x, float y, float z);
matrix_float4x4 matrix_from_perspective_fov_aspectLH(const float fovY, const float aspect,
                                                     const float nearZ, const float farZ);
matrix_float4x4 matrix_for_frustum(const float left, const float right,
                                   const float bottom, const float top,
                                   const float znear, const float zfar);

double degrees_to_radians(double degrees);
double radians_to_degrees(double radians);

#endif /* VROMath_h */
