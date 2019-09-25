//
//  VROSharedStructures.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright (c) 2015 Raj Advani. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef VROSharedStructures_h
#define VROSharedStructures_h

#include "VRODefines.h"
#if VRO_METAL

#include <simd/simd.h>

typedef struct {
    int type;
    vector_float3 position;
    vector_float3 direction;
    vector_float3 color;
    
    float attenuation_start_distance;
    float attenuation_end_distance;
    float attenuation_falloff_exp;
    
    float spot_inner_angle;
    float spot_outer_angle;
} VROLightUniforms;

typedef struct {
    matrix_float4x4 modelview_projection_matrix;
    matrix_float4x4 modelview_matrix;
    matrix_float4x4 model_matrix;
    matrix_float4x4 normal_matrix;
    vector_float3   camera_position;
} VROViewUniforms;

typedef struct {
    vector_float4    diffuse_surface_color;
    float            diffuse_intensity;
    float            shininess;
    float            alpha;
} VROMaterialUniforms;

typedef struct {
    vector_float3    ambient_light_color;
    VROLightUniforms lights[8];
    int              num_lights;
} VROSceneLightingUniforms;

typedef struct {
    float texcoord_scale;
} VRODistortionUniforms;

#endif
#endif /* SharedStructures_h */

