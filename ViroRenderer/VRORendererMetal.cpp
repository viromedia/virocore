//
//  VRORendererMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VRORendererMetal.h"
#include "VRORenderContextMetal.h"
#include "VRODistortionRenderer.h"
#include "VROView.h"

VRORendererMetal::VRORendererMetal(std::shared_ptr<VRODevice> device, VROView *view, VRORenderContext *context) :
    VRORenderer(device, context),
    _view(view) {
    
    _distortionRenderer = new VRODistortionRenderer(device);
    _inflight_semaphore = dispatch_semaphore_create(3);
}

VRORendererMetal::~VRORendererMetal() {
    
}

bool VRORendererMetal::isVignetteEnabled() const {
    return _distortionRenderer->isVignetteEnabled();
}

void VRORendererMetal::setVignetteEnabled(bool vignetteEnabled) {
    _distortionRenderer->setVignetteEnabled(vignetteEnabled);
}

bool VRORendererMetal::isChromaticAberrationCorrectionEnabled() const {
    return _distortionRenderer->isChromaticAberrationEnabled();
}

void VRORendererMetal::setChromaticAberrationCorrectionEnabled(bool enabled) {
    _distortionRenderer->setChromaticAberrationEnabled(enabled);
}

void VRORendererMetal::prepareFrame(const VRORenderContext &context) {
    VRORenderContextMetal &renderContext = (VRORenderContextMetal &)context;
    dispatch_semaphore_wait(_inflight_semaphore, DISPATCH_TIME_FOREVER);
    
    /*
     A single command buffer collects all render events for a frame.
     */
    id <MTLCommandBuffer> commandBuffer = [renderContext.getCommandQueue() commandBuffer];
    commandBuffer.label = @"CommandBuffer";
    
    /*
     When the command buffer is executed by the GPU, signal the semaphor
     (required by the view since it will signal set up the next buffer).
     */
    __block dispatch_semaphore_t block_sema = _inflight_semaphore;
    [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
        dispatch_semaphore_signal(block_sema);
     }];
    
    renderContext.setCommandBuffer(commandBuffer);
}

void VRORendererMetal::endFrame(const VRORenderContext &context) {
    VRORenderContextMetal &renderContext = (VRORenderContextMetal &)context;

    id <MTLCommandBuffer> commandBuffer = renderContext.getCommandBuffer();
    [commandBuffer presentDrawable:_view.currentDrawable];
    [commandBuffer commit];
}

void VRORendererMetal::renderVRDistortion(const VRORenderContext &context) {
    VRORenderContextMetal &renderContext = (VRORenderContextMetal &)context;
    id <MTLCommandBuffer> commandBuffer = renderContext.getCommandBuffer();

    _distortionRenderer->updateDistortion(renderContext.getDevice(), renderContext.getLibrary(), _view);
    
    std::shared_ptr<VRORenderTarget> eyeTarget = _distortionRenderer->bindEyeRenderTarget(commandBuffer);
    renderContext.setRenderTarget(eyeTarget);
    
    id <MTLRenderCommandEncoder> eyeRenderEncoder = eyeTarget->getRenderEncoder();
    
    drawFrame(false);
    [eyeRenderEncoder endEncoding];
    
    std::shared_ptr<VRORenderTarget> screenTarget = std::make_shared<VRORenderTarget>(_view, commandBuffer);
    renderContext.setRenderTarget(screenTarget);
    
    id <MTLRenderCommandEncoder> screenRenderEncoder = screenTarget->getRenderEncoder();
    
    _distortionRenderer->renderEyesToScreen(screenRenderEncoder, renderContext.getFrame());
    [screenRenderEncoder endEncoding];
}

void VRORendererMetal::renderMonocular(const VRORenderContext &context) {
    VRORenderContextMetal &renderContext = (VRORenderContextMetal &)context;
    id <MTLCommandBuffer> commandBuffer = renderContext.getCommandBuffer();

    std::shared_ptr<VRORenderTarget> screenTarget = std::make_shared<VRORenderTarget>(_view, commandBuffer);
    renderContext.setRenderTarget(screenTarget);
    
    drawFrame(true);
    [screenTarget->getRenderEncoder() endEncoding];
}

void VRORendererMetal::onEyesUpdated(VROEye *leftEye, VROEye *rightEye) {
    _distortionRenderer->fovDidChange(leftEye, rightEye,
                                      getVirtualEyeToScreenDistance());
}