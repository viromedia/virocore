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
#include <simd/simd.h>
#include "VRORenderContext.h"
#include "SharedStructures.h"
#include "VRORect.h"
#include <vector>
#include <stack>
#include <memory>

class VROLayer : public std::enable_shared_from_this<VROLayer> {
    
public:
    
    VROLayer();
    virtual ~VROLayer();
    
    void hydrate(const VRORenderContext &context);
    void render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack);
    
    void setFrame(VRORect frame);
    void setBounds(VRORect bounds);
    void setPosition(VROPoint point);
    
    VRORect getFrame() const;
    VRORect getBounds() const;
    VROPoint getPosition() const;
    
    void setBackgroundColor(vector_float4 backgroundColor);
    vector_float4 getBackgroundColor() const;
    
    void addSublayer(std::shared_ptr<VROLayer> &layer);
    void removeFromSuperlayer();
    
private:
    
    id <MTLRenderPipelineState> _pipelineState;
    id <MTLDepthStencilState> _depthState;
    
    id <MTLBuffer> _vertexBuffer;
    id <MTLBuffer> _dynamicConstantBuffer;
    
    VRORect _frame;
    
    vector_float4 _backgroundColor;
    
    std::vector<std::shared_ptr<VROLayer>> _sublayers;
    std::shared_ptr<VROLayer> _superlayer;
    std::shared_ptr<VROLayer> _presentationLayer;
    
    void buildQuad(VROLayerVertexLayout *layout);
    
};

#endif /* VROLayer_h */
