//
//  VROConvert.h
//  ViroRenderer
//
//  Created by Raj Advani on 11/2/16.
//  Copyright © 2016 Viro Media. All rights reserved.
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

#ifndef VROConvert_h
#define VROConvert_h

#include "VRODefines.h"

class VROMatrix4f;
class VROVector3f;
class VROVector4f;
enum class VROCameraOrientation;

#if VRO_PLATFORM_IOS
#include <GLKit/GLKit.h>
#include <simd/simd.h>
#endif

/*
 Utilities for converting from platform-specific objects (iOS
 objects) into generic Viro objects.
 */
class VROConvert {
    
public:
    
#if VRO_PLATFORM_IOS
    static VROMatrix4f toMatrix4f(GLKMatrix4 glm);
    static VROCameraOrientation toCameraOrientation(UIInterfaceOrientation orientation);
    static UIInterfaceOrientation toDeviceOrientation(VROCameraOrientation orientation);
    
    static vector_float3 toVectorFloat3(VROVector3f v);
    static vector_float4 toVectorFloat4(VROVector3f v, float w);
    static vector_float4 toVectorFloat4(VROVector4f v);
    static matrix_float4x4 toMatrixFloat4x4(VROMatrix4f m);
    static VROVector3f toVector3f(vector_float3 v);
    static VROMatrix4f toMatrix4f(matrix_float3x3 m);
    static VROMatrix4f toMatrix4f(matrix_float4x4 m);
#endif
    
};

#endif /* VROConvert_h */
