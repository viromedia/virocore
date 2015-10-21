//
//  VROLayerSubstrateMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/20/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROLayerSubstrateMetal.h"
#include "VRORenderContextMetal.h"
#include "VROLayer.h"
#include "VRORect.h"
#include "VROMath.h"

void VROLayerSubstrateMetal::hydrate(const VRORenderContext &context) {
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
    
    id <MTLDevice> device = metal.getDevice();
    
    _vertexBuffer = [device newBufferWithLength:sizeof(VROLayerVertexLayout) * kCornersInLayer options:0];
    _vertexBuffer.label = @"VROLayerVertexBuffer";
    
    VROLayerVertexLayout *vertexLayout = (VROLayerVertexLayout *)[_vertexBuffer contents];
    buildQuad(vertexLayout);
    
    _uniformsBuffer = [device newBufferWithLength:sizeof(uniforms_t) options:0];
    _uniformsBuffer.label = @"VROLayerUniformBuffer";
    
    id <MTLFunction> fragmentProgram = [metal.getLibrary() newFunctionWithName:@"lighting_fragment"];
    id <MTLFunction> vertexProgram   = [metal.getLibrary() newFunctionWithName:@"lighting_vertex"];
    
    MTLVertexDescriptor *vertexDescriptor = [MTLVertexDescriptor vertexDescriptor];
    vertexDescriptor.attributes[0].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[0].offset = 0;
    vertexDescriptor.attributes[0].bufferIndex = 0;
    
    vertexDescriptor.attributes[1].format = MTLVertexFormatFloat2;
    vertexDescriptor.attributes[1].offset = sizeof(float) * 3;
    vertexDescriptor.attributes[1].bufferIndex = 0;
    
    vertexDescriptor.attributes[2].format = MTLVertexFormatFloat3;
    vertexDescriptor.attributes[2].offset = sizeof(float) * 3 + sizeof(float) * 2;
    vertexDescriptor.attributes[2].bufferIndex = 0;
    
    vertexDescriptor.layouts[0].stepRate = 1;
    vertexDescriptor.layouts[0].stride = sizeof(VROLayerVertexLayout);
    vertexDescriptor.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"VROLayerPipeline";
    pipelineStateDescriptor.sampleCount = metal.getSampleCount();
    pipelineStateDescriptor.vertexFunction = vertexProgram;
    pipelineStateDescriptor.fragmentFunction = fragmentProgram;
    pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = metal.getColorPixelFormat();
    pipelineStateDescriptor.depthAttachmentPixelFormat = metal.getDepthStencilPixelFormat();
    pipelineStateDescriptor.stencilAttachmentPixelFormat = metal.getDepthStencilPixelFormat();
    
    NSError *error = NULL;
    _pipelineState = [metal.getDevice() newRenderPipelineStateWithDescriptor:pipelineStateDescriptor error:&error];
    if (!_pipelineState) {
        NSLog(@"Failed to created pipeline state, error %@", error);
    }
    
    MTLDepthStencilDescriptor *depthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
    depthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;
    depthStateDesc.depthWriteEnabled = NO;
    
    _depthState = [metal.getDevice() newDepthStencilStateWithDescriptor:depthStateDesc];
}

void VROLayerSubstrateMetal::render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack) {
    std::shared_ptr<VROLayer> layer = _layer.lock();
    if (!layer) {
        return;
    }
    
    std::shared_ptr<VROLayer> superlayer = layer->getSuperlayer();
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
    
    VROPoint pt = layer->getPosition();
    VRORect frame = layer->getFrame();
    
    matrix_float4x4 scaleMtx = matrix_from_scale(frame.size.width, frame.size.height, 1.0);
    
    /*
     If the layer is a sublayer, then its coordinate system follows the 2D
     convention of origin top-left, Y down.
     */
    float y = superlayer ? -pt.y : pt.y;
    matrix_float4x4 translationMtx = matrix_from_translation(pt.x, y, pt.z);
    matrix_float4x4 modelMtx = matrix_multiply(translationMtx, scaleMtx);
    
    matrix_float4x4 mvParent = mvStack.top();
    matrix_float4x4 mv = matrix_multiply(mvParent, modelMtx);
    
    // Load constant buffer data into appropriate buffer at current index
    uniforms_t *uniforms = (uniforms_t *)[_uniformsBuffer contents];
    uniforms->normal_matrix = matrix_invert(matrix_transpose(mv));
    uniforms->modelview_projection_matrix = matrix_multiply(metal.getProjectionMatrix(), mv);
    uniforms->diffuse_color = layer->getBackgroundColor();
    
    id <MTLRenderCommandEncoder> renderEncoder = metal.getRenderEncoder();
    
    /*
     Set the buffers and render.
     */
    [renderEncoder pushDebugGroup:@"VROLayer"];
    
    [renderEncoder setDepthStencilState:_depthState];
    [renderEncoder setRenderPipelineState:_pipelineState];
    [renderEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
    [renderEncoder setVertexBuffer:_uniformsBuffer offset:0 atIndex:1 ];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0
                      vertexCount:kCornersInLayer];
    
    [renderEncoder popDebugGroup];
}

matrix_float4x4 VROLayerSubstrateMetal::getChildTransform() {
    std::shared_ptr<VROLayer> layer = _layer.lock();
    if (!layer) {
        return matrix_identity_float4x4;
    }
    
    std::shared_ptr<VROLayer> superlayer = layer->getSuperlayer();
    VRORect frame = layer->getFrame();
    
    float parentOriginY = superlayer ? -frame.origin.y : frame.origin.y + frame.size.height;
    return matrix_from_translation(frame.origin.x, parentOriginY, frame.origin.z);
}
