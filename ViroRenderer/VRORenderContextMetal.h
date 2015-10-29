//
//  VRORenderContextMetal.h
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRORenderContextMetal_h
#define VRORenderContextMetal_h

#include <stdio.h>
#include <Metal/Metal.h>
#include <MetalKit/MetalKit.h>
#include "VRORenderContext.h"

/*
 Render context for Metal.
 */
class VRORenderContextMetal : public VRORenderContext {
    
public:
    
    VRORenderContextMetal(MTKView *view,
                          id <MTLDevice> device) {
        
        _device = device;
        _commandQueue = [device newCommandQueue];
        _library = [device newDefaultLibrary];
        
        _colorPixelFormat = view.colorPixelFormat;
        _depthStencilPixelFormat = view.depthStencilPixelFormat;
        _sampleCount = view.sampleCount;
    }
    
    void setCommandBuffer(id <MTLCommandBuffer> commandBuffer) {
        _commandBuffer = commandBuffer;
    }
    void setRenderEncoder(id <MTLRenderCommandEncoder> renderEncoder) {
        _renderEncoder = renderEncoder;
    }
    void setProjectionMatrix(matrix_float4x4 projectionMatrix) {
        _projectionMatrix = projectionMatrix;
    }
    void setViewMatrix(matrix_float4x4 viewMatrix) {
        _viewMatrix = viewMatrix;
    }
    
    id <MTLDevice> getDevice() const {
        return _device;
    }
    id <MTLCommandQueue> getCommandQueue() const {
        return _commandQueue;
    }
    id <MTLLibrary> getLibrary() const {
        return _library;
    }
    
    id <MTLCommandBuffer> getCommandBuffer() const {
        return _commandBuffer;
    }
    id <MTLRenderCommandEncoder> getRenderEncoder() const {
        return _renderEncoder;
    }
    
    MTLPixelFormat getColorPixelFormat() const {
        return _colorPixelFormat;
    }
    MTLPixelFormat getDepthStencilPixelFormat() const {
        return _depthStencilPixelFormat;
    }
    NSUInteger getSampleCount() const {
        return _sampleCount;
    }
    
    matrix_float4x4 getProjectionMatrix() const {
        return _projectionMatrix;
    }
    matrix_float4x4 getViewMatrix() const {
        return _viewMatrix;
    }
    
private:
    
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;
    id <MTLLibrary> _library;
    
    MTLPixelFormat _colorPixelFormat;
    MTLPixelFormat _depthStencilPixelFormat;
    NSUInteger _sampleCount;
    
    id <MTLCommandBuffer> _commandBuffer;
    id <MTLRenderCommandEncoder> _renderEncoder;
    
    matrix_float4x4 _projectionMatrix;
    matrix_float4x4 _viewMatrix;
    
};

#endif /* VRORenderContextMetal_h */
