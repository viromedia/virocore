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
#include "VRORenderTarget.h"
#include "VROMatrix4f.h"
#include <memory>

/*
 Render context for Metal.
 */
class VRORenderContextMetal : public VRORenderContext {
    
public:
    
    VRORenderContextMetal(id <MTLDevice> device) {
        _device = device;
        _commandQueue = [device newCommandQueue];
        _library = [device newDefaultLibrary];
    }
    
    void setRenderTarget(std::shared_ptr<VRORenderTarget> renderTarget) {
        _renderTarget = renderTarget;
    }
    void setProjectionMatrix(VROMatrix4f projectionMatrix) {
        _projectionMatrix = projectionMatrix;
    }
    void setViewMatrix(VROMatrix4f viewMatrix) {
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
    std::shared_ptr<VRORenderTarget> getRenderTarget() const {
        return _renderTarget;
    }
    
    VROMatrix4f getProjectionMatrix() const {
        return _projectionMatrix;
    }
    VROMatrix4f getViewMatrix() const {
        return _viewMatrix;
    }
    
private:
    
    id <MTLDevice> _device;
    id <MTLCommandQueue> _commandQueue;
    id <MTLLibrary> _library;
    
    std::shared_ptr<VRORenderTarget> _renderTarget;
    
    VROMatrix4f _projectionMatrix;
    VROMatrix4f _viewMatrix;
    
};

#endif /* VRORenderContextMetal_h */
