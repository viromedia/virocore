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

VROLayerSubstrateMetal::VROLayerSubstrateMetal(const VRORenderContext &context) :
    VROLayerSubstrate(),
    _device(((VRORenderContextMetal &) context).getDevice()) {

}


void VROLayerSubstrateMetal::setContents(const void *data, const size_t dataLength, int width, int height) {
    int bytesPerPixel = 4;
    MTLTextureDescriptor *descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                          width:width height:height mipmapped:NO];
    _texture = [_device newTextureWithDescriptor:descriptor];
    
    MTLRegion region = MTLRegionMake2D(0, 0, width, height);
    [_texture replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:bytesPerPixel * width];
}

void VROLayerSubstrateMetal::hydrate(const VRORenderContext &context) {
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
    
    _vertexBuffer = [_device newBufferWithLength:sizeof(VROLayerVertexLayout) * kCornersInLayer options:0];
    _vertexBuffer.label = @"VROLayerVertexBuffer";
    
    VROLayerVertexLayout *vertexLayout = (VROLayerVertexLayout *)[_vertexBuffer contents];
    buildQuad(vertexLayout);
    
    _uniformsBuffer = [_device newBufferWithLength:sizeof(uniforms_t) options:0];
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
    
    std::shared_ptr<VRORenderTarget> renderTarget = metal.getRenderTarget();
    
    MTLRenderPipelineDescriptor *pipelineStateDescriptor = [[MTLRenderPipelineDescriptor alloc] init];
    pipelineStateDescriptor.label = @"VROLayerPipeline";
    pipelineStateDescriptor.sampleCount = renderTarget->getSampleCount();
    pipelineStateDescriptor.vertexFunction = vertexProgram;
    pipelineStateDescriptor.fragmentFunction = fragmentProgram;
    pipelineStateDescriptor.vertexDescriptor = vertexDescriptor;
    pipelineStateDescriptor.colorAttachments[0].pixelFormat = renderTarget->getColorPixelFormat();
    pipelineStateDescriptor.depthAttachmentPixelFormat = renderTarget->getDepthStencilPixelFormat();
    pipelineStateDescriptor.stencilAttachmentPixelFormat = renderTarget->getDepthStencilPixelFormat();
    
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

void VROLayerSubstrateMetal::render(const VRORenderContext &context,
                                    matrix_float4x4 mv,
                                    vector_float4 bgColor) {
    
    if (!_pipelineState) {
        hydrate(context);
    }
    
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
    
    uniforms_t *uniforms = (uniforms_t *)[_uniformsBuffer contents];
    uniforms->normal_matrix = matrix_invert(matrix_transpose(mv));
    uniforms->modelview_projection_matrix = matrix_multiply(metal.getProjectionMatrix(), mv);
    uniforms->diffuse_color = bgColor;
    
    id <MTLRenderCommandEncoder> renderEncoder = metal.getRenderTarget()->getRenderEncoder();
    
    [renderEncoder pushDebugGroup:@"VROLayer"];
    
    [renderEncoder setDepthStencilState:_depthState];
    [renderEncoder setRenderPipelineState:_pipelineState];
    [renderEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
    [renderEncoder setVertexBuffer:_uniformsBuffer offset:0 atIndex:1];
    [renderEncoder setFragmentTexture:_texture atIndex:0];
    
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:kCornersInLayer];
    [renderEncoder popDebugGroup];
}
