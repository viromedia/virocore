//
//  VROLayer.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include "VROLayer.h"
#include "VRORenderContextMetal.h"
#include "VROMath.h"

static const size_t kMaxBytesPerFrame = 1024*1024;
static const size_t kCornersInLayer = 6;

#pragma mark - Initialization

VROLayer::VROLayer() {

}

VROLayer::~VROLayer() {
    
}

void VROLayer::buildQuad(VROLayerVertexLayout *vertexLayout) {
    const float x = 1;
    const float y = 1;
    const float z = 1;
    
    vertexLayout[0].x = 0;
    vertexLayout[0].y = 0;
    vertexLayout[0].z = z;
    vertexLayout[0].u = 0;
    vertexLayout[0].v = 0;
    vertexLayout[0].nx = 0;
    vertexLayout[0].ny = 0;
    vertexLayout[0].nz = -1;
    
    vertexLayout[1].x = x;
    vertexLayout[1].y = 0;
    vertexLayout[1].z = z;
    vertexLayout[1].u = 1;
    vertexLayout[1].v = 0;
    vertexLayout[1].nx = 0;
    vertexLayout[1].ny = 0;
    vertexLayout[1].nz = -1;
    
    vertexLayout[2].x = 0;
    vertexLayout[2].y = y;
    vertexLayout[2].z = z;
    vertexLayout[2].u = 0;
    vertexLayout[2].v = 1;
    vertexLayout[2].nx = 0;
    vertexLayout[2].ny = 0;
    vertexLayout[2].nz = -1;
    
    vertexLayout[3].x = x;
    vertexLayout[3].y = y;
    vertexLayout[3].z = z;
    vertexLayout[3].u = 1;
    vertexLayout[3].v = 1;
    vertexLayout[3].nx = 0;
    vertexLayout[3].ny = 0;
    vertexLayout[3].nz = -1;
    
    vertexLayout[4].x = 0;
    vertexLayout[4].y = y;
    vertexLayout[4].z = z;
    vertexLayout[4].u = 0;
    vertexLayout[4].v = 1;
    vertexLayout[4].nx = 0;
    vertexLayout[4].ny = 0;
    vertexLayout[4].nz = -1;
    
    vertexLayout[5].x = x;
    vertexLayout[5].y = 0;
    vertexLayout[5].z = z;
    vertexLayout[5].u = 1;
    vertexLayout[5].v = 0;
    vertexLayout[5].nx = 0;
    vertexLayout[5].ny = 0;
    vertexLayout[5].nz = -1;
}

#pragma mark - Rendering

void VROLayer::hydrate(const VRORenderContext &context) {
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
    
    id <MTLDevice> device = metal.getDevice();
    
    _vertexBuffer = [device newBufferWithLength:sizeof(VROLayerVertexLayout) * kCornersInLayer options:0];
    _vertexBuffer.label = @"VROLayerVertexBuffer";
    
    VROLayerVertexLayout *vertexLayout = (VROLayerVertexLayout *)[_vertexBuffer contents];
    buildQuad(vertexLayout);
    
    _dynamicConstantBuffer = [device newBufferWithLength:kMaxBytesPerFrame options:0];
    _dynamicConstantBuffer.label = @"VROLayerUniformBuffer";

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
    depthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
    depthStateDesc.depthWriteEnabled = YES;
    
    _depthState = [metal.getDevice() newDepthStencilStateWithDescriptor:depthStateDesc];
}

void VROLayer::render(const VRORenderContext &context, std::stack<matrix_float4x4> mvStack) {
    const VRORenderContextMetal &metal = (VRORenderContextMetal &)context;
    
    VROPoint pt = getPosition();
    matrix_float4x4 model = matrix_from_translation(pt.x, pt.y, 10.0f);
    
    matrix_float4x4 mvParent = mvStack.top();
    matrix_float4x4 mv = matrix_multiply(mvParent, model);
    
    // Load constant buffer data into appropriate buffer at current index
    uniforms_t *uniforms = &((uniforms_t *)[_dynamicConstantBuffer contents])[metal.getConstantDataBufferIndex()];
    uniforms->normal_matrix = matrix_invert(matrix_transpose(mv));
    uniforms->modelview_projection_matrix = matrix_multiply(metal.getProjectionMatrix(), mv);
    uniforms->diffuse_color = _backgroundColor;
    
    id <MTLRenderCommandEncoder> renderEncoder = metal.getRenderEncoder();
    
    /*
     Set the buffers and render.
     */
    [renderEncoder pushDebugGroup:@"VROLayer"];
    
    [renderEncoder setDepthStencilState:_depthState];
    [renderEncoder setRenderPipelineState:_pipelineState];
    [renderEncoder setVertexBuffer:_vertexBuffer offset:0 atIndex:0];
    [renderEncoder setVertexBuffer:_dynamicConstantBuffer offset:(sizeof(uniforms_t) * metal.getConstantDataBufferIndex()) atIndex:1 ];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:kCornersInLayer];
    
    [renderEncoder popDebugGroup];
    
    /*
     Now render the children.
     */
    mvStack.push(mv);
    for (std::shared_ptr<VROLayer> childLayer : _sublayers) {
        childLayer->render(context, mvStack);
    }
    mvStack.pop();
}

#pragma mark - Layer Properties

void VROLayer::setBackgroundColor(vector_float4 backgroundColor) {
    _backgroundColor = backgroundColor;
}

vector_float4 VROLayer::getBackgroundColor() const {
    return _backgroundColor;
}

#pragma mark - Spatial Position

void VROLayer::setFrame(VRORect frame) {
    _frame = frame;
}

void VROLayer::setBounds(VRORect bounds) {
    _frame.size = bounds.size;
}

void VROLayer::setPosition(VROPoint point) {
    _frame.origin.x = point.x - _frame.size.width  / 2.0f;
    _frame.origin.y = point.y - _frame.size.height / 2.0f;
}

VRORect VROLayer::getFrame() const {
    return _frame;
}

VRORect VROLayer::getBounds() const {
    return {{0, 0}, {_frame.size.width, _frame.size.height}};
}

VROPoint VROLayer::getPosition() const {
    return {_frame.origin.x + _frame.size.width  / 2.0f,
            _frame.origin.y + _frame.size.height / 2.0f };
}

#pragma mark - Layer Tree

void VROLayer::addSublayer(std::shared_ptr<VROLayer> &layer) {
    _sublayers.push_back(layer);
}

void VROLayer::removeFromSuperlayer() {
    std::vector<std::shared_ptr<VROLayer>> parentSublayers = _superlayer->_sublayers;
    parentSublayers.erase(
                          std::remove_if(parentSublayers.begin(), parentSublayers.end(),
                                         [this](std::shared_ptr<VROLayer> layer) {
                                             return layer.get() == this;
                                         }), parentSublayers.end());
}
