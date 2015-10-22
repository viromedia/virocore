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
    void render(const VRORenderContext &context, matrix_float4x4 mv);
    
    void setContents(const void *data, size_t dataLength, int width, int height);
    
private:
    
    id <MTLDevice> _device;
    
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    
    id <MTLBuffer> _vertexBuffer;
    id <MTLBuffer> _uniformsBuffer;
    id <MTLTexture> _texture;
    
};

#endif /* VROLayerSubstrateMetal_h */
