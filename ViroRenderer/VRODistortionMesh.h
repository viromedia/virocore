//
//  VRODistortionMesh.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/29/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRODistortionMesh_h
#define VRODistortionMesh_h

#include <MetalKit/MetalKit.h>
#include "VRODistortion.h"

/*
 The meshes onto which we render each eye to perform barrel distortion.
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

#endif /* VRODistortionMesh_h */
