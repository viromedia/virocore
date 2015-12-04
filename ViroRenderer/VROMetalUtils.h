//
//  VROMetalUtils.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROMetalUtils_h
#define VROMetalUtils_h

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>
#import <simd/simd.h>

class VROVector4f;
class VROMatrix4f;

vector_float4 toVectorFloat4(VROVector4f v);
matrix_float4x4 toMatrixFloat4x4(VROMatrix4f m);

id <MTLTexture> getBlankTexture(id <MTLDevice> device);

#endif /* VROMetalUtils_h */
