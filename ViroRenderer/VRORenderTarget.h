//
//  VRORenderTarget.hpp
//  ViroRenderer
//
//  Created by Raj Advani on 10/30/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#ifndef VRORenderTarget_hpp
#define VRORenderTarget_hpp

#include <MetalKit/MetalKit.h>

/*
 Render targets consist of a bound render encoder (which will receive all
 rendering commands to submit to the target), and the associated view
 parameters.
 */
class VRORenderTarget {
    
public:
    
    /*
     Build a new VRORenderTarget for rendering to the given MTKView.
     Convenience function.
     */
    VRORenderTarget(MTKView *view, id <MTLCommandBuffer> commandBuffer) :
        _colorPixelFormat(view.colorPixelFormat),
        _depthStencilPixelFormat(view.depthStencilPixelFormat),
        _sampleCount(view.sampleCount) {
    
        _renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:view.currentRenderPassDescriptor];
        _renderEncoder.label = @"ScreenRenderEncoder";
    }

    /*
     Build a new VRORenderTarget with the given encoder and view properties.
     */
    VRORenderTarget(id <MTLRenderCommandEncoder> renderEncoder,
                    MTLPixelFormat colorPixelFormat,
                    MTLPixelFormat depthStencilPixelFormat,
                    NSUInteger sampleCount) :
        _renderEncoder(renderEncoder),
        _colorPixelFormat(colorPixelFormat),
        _depthStencilPixelFormat(depthStencilPixelFormat),
        _sampleCount(sampleCount)
    {}
    
    id <MTLRenderCommandEncoder> getRenderEncoder() {
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
    
private:
    
    id <MTLRenderCommandEncoder> _renderEncoder;
    const MTLPixelFormat _colorPixelFormat;
    const MTLPixelFormat _depthStencilPixelFormat;
    const NSUInteger _sampleCount;
    
};

#endif /* VRORenderTarget_hpp */
