//
//  VROLayerSubstrateMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/20/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VROLayerSubstrateMetal_h
#define VROLayerSubstrateMetal_h

#include "VROLayerSubstrate.h"
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>

class VROLayerSubstrateMetal : public VROLayerSubstrate {
    
public:
    
    VROLayerSubstrateMetal(std::shared_ptr<VROLayer> layer) :
        VROLayerSubstrate(layer)
    {}
    virtual ~VROLayerSubstrateMetal() {}
    
    void hydrate(const VRORenderContext &context);
    void render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack);
    matrix_float4x4 getChildTransform();
    
private:
    
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    
    id <MTLBuffer> _vertexBuffer;
    id <MTLBuffer> _uniformsBuffer;
    id <MTLBuffer> _textureBuffer;
    
};

#endif /* VROLayerSubstrateMetal_h */
