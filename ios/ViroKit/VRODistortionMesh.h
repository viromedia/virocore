//
//  VRODistortionMesh.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/29/15.
//  Copyright © 2015 Viro Media. All rights reserved.
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

#ifndef VRODistortionMesh_h
#define VRODistortionMesh_h

#include "VRODefines.h"
#if VRO_METAL

#include "VRODistortion.h"
#include <MetalKit/MetalKit.h>

/*
 The mesh onto which we render each eye to perform barrel distortion.
 */
class VRODistortionMesh {
    
public:
    
    VRODistortionMesh(const VRODistortion distortionRed,
                      const VRODistortion distortionGreen,
                      const VRODistortion distortionBlue,
                      float screenWidth, float screenHeight,
                      float xEyeOffsetScreen, float yEyeOffsetScreen,
                      float textureWidth, float textureHeight,
                      float xEyeOffsetTexture, float yEyeOffsetTexture,
                      float viewportXTexture, float viewportYTexture,
                      float viewportWidthTexture,
                      float viewportHeightTexture,
                      bool vignetteEnabled,
                      id <MTLDevice> gpu);
    
    void render(id <MTLRenderCommandEncoder> renderEncoder) const;

private:

    id <MTLBuffer> _vertexBuffer;
    id <MTLBuffer> _indexBuffer;

};

#endif /* VRO_METAL */
#endif /* VRODistortionMesh_h */
