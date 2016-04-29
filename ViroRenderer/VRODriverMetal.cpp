//
//  VRODriverMetal.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 4/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VRODriverMetal.h"
#include "VRODriverContextMetal.h"
#include "VRODistortionRenderer.h"
#include "VROView.h"
#include "VROEye.h"
#include "VRORenderer.h"
#include "VROHeadTracker.h"
#include "VRODevice.h"
#include "VROViewMetal.h"

static const float zNear = 0.1;
static const float zFar  = 100;

VRODriverMetal::VRODriverMetal(std::shared_ptr<VRORenderer> renderer, id <MTLDevice> device,
                               VROViewMetal *view) :
    _frame(0),
    _renderer(renderer),
    _vrModeEnabled(true),
    _projectionChanged(true),
    _view(view),
    _device(std::make_shared<VRODevice>([UIScreen mainScreen])) {
        
    _context = std::make_shared<VRODriverContextMetal>(device);
    _distortionRenderer = new VRODistortionRenderer(_device);
    _inflight_semaphore = dispatch_semaphore_create(3);
        
    _monocularEye = new VROEye(VROEyeType::Monocular);
    _leftEye = new VROEye(VROEyeType::Left);
    _rightEye = new VROEye(VROEyeType::Right);
        
    _headTracker = new VROHeadTracker();
    _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
}

VRODriverMetal::~VRODriverMetal() {
    delete (_distortionRenderer);
    delete (_headTracker);
    delete (_monocularEye);
    delete (_leftEye);
    delete (_rightEye);
}

#pragma mark - Settings

void VRODriverMetal::onOrientationChange(UIInterfaceOrientation orientation) {
    _headTracker->updateDeviceOrientation(orientation);
}

float VRODriverMetal::getVirtualEyeToScreenDistance() const {
    return _device->getScreenToLensDistance();
}

VROViewport VRODriverMetal::getViewport(VROEyeType type) {
    return getEye(type)->getViewport();
}

VROFieldOfView VRODriverMetal::getFOV(VROEyeType type) {
    return getEye(type)->getFOV();
}

VROEye *VRODriverMetal::getEye(VROEyeType type) {
    switch (type) {
        case VROEyeType::Left:
            return _leftEye;
        case VROEyeType::Right:
            return _rightEye;
        default:
            return _monocularEye;
    }
}

#pragma mark - Render Loop

void VRODriverMetal::driveFrame() {
    if (!_headTracker->isReady()) {
        return;
    }
    
    @autoreleasepool {
        dispatch_semaphore_wait(_inflight_semaphore, DISPATCH_TIME_FOREVER);
        
        /*
         A single command buffer collects all render events for a frame.
         */
        VRODriverContextMetal *driverContext = (VRODriverContextMetal *)_context.get();

        id <MTLCommandBuffer> commandBuffer = [driverContext->getCommandQueue() commandBuffer];
        commandBuffer.label = @"CommandBuffer";
        
        /*
         When the command buffer is executed by the GPU, signal the semaphor
         (required by the view since it will signal set up the next buffer).
         */
        __block dispatch_semaphore_t block_sema = _inflight_semaphore;
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            dispatch_semaphore_signal(block_sema);
        }];

        calculateFrameParameters();
        renderVRDistortion(_frame, commandBuffer);

        [commandBuffer presentDrawable:_view.currentDrawable];
        [commandBuffer commit];        
    }
    
    ++_frame;
}

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

#pragma mark - View Computation

void VRODriverMetal::calculateFrameParameters() {
    if (_projectionChanged) {
        const VROScreen &screen = _device->getScreen();
        _monocularEye->setViewport(0, 0, screen.getWidth(), screen.getHeight());
        
        if (!_vrModeEnabled) {
            updateMonocularEye();
        }
        else {
            updateLeftRightEyes();
        }
        
        _projectionChanged = NO;
    }
}

void VRODriverMetal::updateMonocularEye() {
    const VROScreen &screen = _device->getScreen();
    const float monocularBottomFov = 22.5f;
    const float monocularLeftFov = GLKMathRadiansToDegrees(
                                                           atanf(
                                                                 tanf(GLKMathDegreesToRadians(monocularBottomFov))
                                                                 * screen.getWidthInMeters()
                                                                 / screen.getHeightInMeters()));
    _monocularEye->setFOV(monocularLeftFov, monocularLeftFov, monocularBottomFov, monocularBottomFov,
                          zNear, zFar);
}

void VRODriverMetal::updateLeftRightEyes() {
    const VROScreen &screen = _device->getScreen();
    
    const VRODistortion &distortion = _device->getDistortion();
    float eyeToScreenDistance = getVirtualEyeToScreenDistance();
    
    float outerDistance = (screen.getWidthInMeters() - _device->getInterLensDistance() ) / 2.0f;
    float innerDistance = _device->getInterLensDistance() / 2.0f;
    float bottomDistance = _device->getVerticalDistanceToLensCenter() - screen.getBorderSizeInMeters();
    float topDistance = screen.getHeightInMeters() + screen.getBorderSizeInMeters() - _device->getVerticalDistanceToLensCenter();
    
    float outerAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(outerDistance / eyeToScreenDistance)));
    float innerAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(innerDistance / eyeToScreenDistance)));
    float bottomAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(bottomDistance / eyeToScreenDistance)));
    float topAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(topDistance / eyeToScreenDistance)));
    
    _leftEye->setFOV(MIN(outerAngle,  _device->getMaximumLeftEyeFOV().getLeft()),
                     MIN(innerAngle,  _device->getMaximumLeftEyeFOV().getRight()),
                     MIN(bottomAngle, _device->getMaximumLeftEyeFOV().getBottom()),
                     MIN(topAngle,    _device->getMaximumLeftEyeFOV().getTop()),
                     zNear, zFar);
    
    const VROFieldOfView &leftEyeFov = _leftEye->getFOV();
    _rightEye->setFOV(leftEyeFov.getRight(),
                      leftEyeFov.getLeft(),
                      leftEyeFov.getBottom(),
                      leftEyeFov.getTop(),
                      zNear, zFar);
    
    _distortionRenderer->fovDidChange(_leftEye, _rightEye,
                                      getVirtualEyeToScreenDistance());
}