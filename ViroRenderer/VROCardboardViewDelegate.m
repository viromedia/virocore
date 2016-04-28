//
//  VROCardboardViewDelegate.m
//  ViroRenderer
//
//  Created by Raj Advani on 4/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROCardboardViewDelegate.h"

@interface VROCardboardViewDelegate () {
    
    std::shared_ptr<VRORenderer> _renderer;
    
}

@end

@implementation VROCardboardViewDelegate

- (id)initWithRenderer:(std::shared_ptr<VRORenderer>)renderer {
    self = [super init];
    if (self) {
        _renderer = renderer;
    }
    
    return self;
}

- (void)cardboardView:(GCSCardboardView *)cardboardView didFireEvent:(GCSUserEvent)event {
    
}

/**
 * Called before the first draw frame call. This is called on the GL thread and can be used to do
 * any pre-rendering setup required while on the GL thread.
 */
- (void)cardboardView:(GCSCardboardView *)cardboardView
     willStartDrawing:(GCSHeadTransform *)headTransform {
    
}

/**
 * Called at the start of each frame, before calling both eyes. Delegate should use initialize or
 * clear the GL state. This method is called on the GL thread.
 */
- (void)cardboardView:(GCSCardboardView *)cardboardView
     prepareDrawFrame:(GCSHeadTransform *)headTransform {
    
}

/**
 * Called on each frame to perform the required GL rendering. Delegate should set the GL viewport
 * and scissor it to the viewport returned from |GCSHeadTransforms|'s |viewportForEye| method.
 * This method is called on the GL thread.
 */
- (void)cardboardView:(GCSCardboardView *)cardboardView
              drawEye:(GCSEye)eye
    withHeadTransform:(GCSHeadTransform *)headTransform {
    
}

/*
void VRODriverMetal::driveFrame() {
    
    
    
        VRODriverContextMetal *driverContext = (VRODriverContextMetal *)_context.get();
        
        id <MTLCommandBuffer> commandBuffer = [driverContext->getCommandQueue() commandBuffer];
        commandBuffer.label = @"CommandBuffer";
        
        calculateFrameParameters();
        
        VROTransaction::beginImplicitAnimation();
        VROTransaction::update();
        
        renderVRDistortion(_frame, commandBuffer);
        
        [commandBuffer presentDrawable:_view.currentDrawable];
        [commandBuffer commit];
        
        VROTransaction::commitAll();
    
    ++_frame;
}
 */
/*
void VRODriverMetal::renderVRDistortion(int frame, id <MTLCommandBuffer> commandBuffer) {
    VRODriverContextMetal *driverContext = (VRODriverContextMetal *)_context.get();
    _distortionRenderer->updateDistortion(driverContext->getDevice(), driverContext->getLibrary(), _view);
    
    std::shared_ptr<VRORenderTarget> eyeTarget = _distortionRenderer->bindEyeRenderTarget(commandBuffer);
    driverContext->setRenderTarget(eyeTarget);
    
    VROMatrix4f headRotation = _headTracker->getHeadRotation();
    _renderer->prepareFrame(frame, headRotation, *driverContext);
    
    float halfLensDistance = _device->getInterLensDistance() * 0.5f;
    VROMatrix4f leftEyeMatrix  = matrix_from_translation( halfLensDistance, 0, 0);
    VROMatrix4f rightEyeMatrix = matrix_from_translation(-halfLensDistance, 0, 0);
    
    id <MTLRenderCommandEncoder> eyeRenderEncoder = eyeTarget->getRenderEncoder();
    [eyeRenderEncoder setViewport:_leftEye->getViewport().toMetalViewport()];
    [eyeRenderEncoder setScissorRect:_leftEye->getViewport().toMetalScissor()];
    
    _renderer->renderEye(_leftEye->getType(), leftEyeMatrix, _leftEye->getPerspectiveMatrix(),
                         *driverContext);
    
    [eyeRenderEncoder setViewport:_rightEye->getViewport().toMetalViewport()];
    [eyeRenderEncoder setScissorRect:_rightEye->getViewport().toMetalScissor()];
    
    _renderer->renderEye(_rightEye->getType(), rightEyeMatrix, _rightEye->getPerspectiveMatrix(),
                         *driverContext);
    _renderer->endFrame(*driverContext);
    
    [eyeRenderEncoder endEncoding];
    
    std::shared_ptr<VRORenderTarget> screenTarget = std::make_shared<VRORenderTarget>(_view, commandBuffer);
    id <MTLRenderCommandEncoder> screenRenderEncoder = screenTarget->getRenderEncoder();
    
    _distortionRenderer->renderEyesToScreen(screenRenderEncoder, frame);
    [screenRenderEncoder endEncoding];
}
 */

@end
