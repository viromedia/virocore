//
//  VROSharedStructures.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright (c) 2015 Raj Advani. All rights reserved.
//

#ifndef VROSharedStructures_h
#define VROSharedStructures_h

#include <simd/simd.h>

typedef struct {
    vector_float4 position;
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
    float            shininess;
} VROMaterialUniforms;

typedef struct {
    vector_float3    ambient_light_color;
    VROLightUniforms lights[8];
    int              num_lights;
} VROSceneLightingUniforms;

typedef struct {
    float texcoord_scale;
} VRODistortionUniforms;

#endif /* SharedStructures_h */

