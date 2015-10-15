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
} uniforms_t;

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

#endif /* SharedStructures_h */

