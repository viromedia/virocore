//
//  VROConvert.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROConvert_h
#define VROConvert_h

#include "VRODefines.h"

class VROMatrix4f;
class VROVector3f;
class VROVector4f;
enum class VROCameraOrientation;

#include <simd/simd.h>

/*
 Utilities for converting from platform-specific objects (iOS
 objects) into generic Viro objects.
 */
class VROConvert {
    
public:
    
    static vector_float3 toVectorFloat3(VROVector3f v);
    static vector_float4 toVectorFloat4(VROVector3f v, float w);
    static vector_float4 toVectorFloat4(VROVector4f v);
    static matrix_float4x4 toMatrixFloat4x4(VROMatrix4f m);
    static VROVector3f toVector3f(vector_float3 v);
    static VROMatrix4f toMatrix4f(matrix_float3x3 m);
    static VROMatrix4f toMatrix4f(matrix_float4x4 m);
    
};

#endif /* VROConvert_h */
