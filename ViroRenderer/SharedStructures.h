//
//  SharedStructures.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright (c) 2015 Raj Advani. All rights reserved.
//

#ifndef SharedStructures_h
#define SharedStructures_h

#include <simd/simd.h>

typedef struct {
    matrix_float4x4 modelview_projection_matrix;
    matrix_float4x4 normal_matrix;
    vector_float4   diffuse_color;
} uniforms_t;

typedef struct {
    matrix_float4x4 modelview_projection_matrix;
    matrix_float4x4 normal_matrix;
    
    vector_float4   ambient_color;
    vector_float4   diffuse_color;
} VROConstantLightingUniforms;

typedef struct {
    matrix_float4x4 modelview_projection_matrix;
    matrix_float4x4 normal_matrix;
    
    vector_float4   ambient_color;
    vector_float4   diffuse_color;
    vector_float4   specular_color;
    
    float           shininess;
    float           fresnel;
} VROBlinnLightingUniforms;

typedef struct {
    matrix_float4x4 modelview_projection_matrix;
    matrix_float4x4 normal_matrix;
    
    vector_float4   ambient_color;
    vector_float4   diffuse_color;
    vector_float4   specular_color;
    
    float           shininess;
    float           fresnel;
} VROPhongLightingUniforms;

typedef struct {
    matrix_float4x4 modelview_projection_matrix;
    matrix_float4x4 normal_matrix;
    
    vector_float4   ambient_color;
    vector_float4   diffuse_color;
    vector_float4   specular_color;
    
    float           shininess;
    float           fresnel;
} VROLambertLightingUniforms;

typedef struct {
    float x;
    float y;
    float z;
    float u;
    float v;
    float nx;
    float ny;
    float nz;
} VROLayerVertexLayout;

typedef struct {
    float texcoord_scale;
} VRODistortionUniforms;

#endif /* SharedStructures_h */

