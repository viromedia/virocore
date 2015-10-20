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
matrix_float4x4 matrix_from_perspective_fov_aspectLH(const float fovY, const float aspect, const float nearZ, const float farZ);

#endif /* VROMath_h */
