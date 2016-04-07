//
//  VROMetalUtils.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMetalUtils_h
#define VROMetalUtils_h

#import <simd/simd.h>

class VROVector3f;
class VROVector4f;
class VROMatrix4f;

vector_float3 toVectorFloat3(VROVector3f v);
vector_float4 toVectorFloat4(VROVector3f v, float w);
vector_float4 toVectorFloat4(VROVector4f v);
matrix_float4x4 toMatrixFloat4x4(VROMatrix4f m);

#endif /* VROMetalUtils_h */
