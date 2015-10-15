//
//  VROLayer.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROLayer_h
#define VROLayer_h

#include <stdio.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include "VRORenderContext.h"
#include "SharedStructures.h"

class VROLayer {
    
public:
    
    VROLayer();
    virtual ~VROLayer();
    
    void hydrate(const VRORenderContext &context);
    void render(const VRORenderContext &context);
    
private:
    
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    
    id <MTLBuffer> _vertexBuffer;
    id <MTLBuffer> _dynamicConstantBuffer;
    
    void buildQuad(VROLayerVertexLayout *layout);
    
};

#endif /* VROLayer_h */
