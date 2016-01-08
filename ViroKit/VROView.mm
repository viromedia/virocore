//
//  VROView.m
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import "VROView.h"
#import "VRODevice.h"
#import "VRODistortion.h"
#import "VRODistortionRenderer.h"
#import "VROEye.h"
#import "VROFieldOfView.h"
#import "VROViewport.h"
#import "VROScreen.h"
#import "VROHeadTracker.h"
#import "VROMagnetSensor.h"
#import "VRORenderContextMetal.h"
#import "VROTransaction.h"
#import "VROImageUtil.h"

@interface VROView () {
    VROMagnetSensor *_magnetSensor;
    VROHeadTracker *_headTracker;
    VRODevice *_device;
    
    VROEye *_monocularEye;
    VROEye *_leftEye;
    VROEye *_rightEye;
    
    VRODistortionRenderer *_distortionRenderer;
    
    VRORenderContextMetal *_renderContext;
    dispatch_semaphore_t _inflight_semaphore;
    
    float _distortionCorrectionScale;
    
    float _zNear;
    float _zFar;
    
    BOOL _projectionChanged;
    BOOL _frameParamentersReady;
    BOOL _rendererInitialized;
}

@end

@implementation VROView

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (void)initRenderer {
    // Do not allow the display to go into sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    _magnetSensor = new VROMagnetSensor();
    _headTracker = new VROHeadTracker();
    _device = new VRODevice([UIScreen mainScreen]);
    
    _monocularEye = new VROEye(VROEye::TypeMonocular);
    _leftEye = new VROEye(VROEye::TypeLeft);
    _rightEye = new VROEye(VROEye::TypeRight);
    
    _distortionRenderer = new VRODistortionRenderer(*_device);
    _distortionCorrectionScale = 1.0f;
    
    _vrModeEnabled = YES;
    _distortionCorrectionEnabled = YES;
    _rendererInitialized = NO;
    
    _zNear = 0.1f;
    _zFar = 100.0f;
    
    _projectionChanged = YES;
    _frameParamentersReady = NO;
    
    _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
    _magnetSensor->start();
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(magneticTriggerPressed:)
                                                 name:VROTriggerPressedNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    
    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    
    self.device = device;
    self.delegate = self;
    self.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;
    
    _inflight_semaphore = dispatch_semaphore_create(3);
    _renderContext = new VRORenderContextMetal(device);
    initBlankTexture(*_renderContext);
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [self.renderDelegate shutdownRendererWithView:self];
    
    delete (_magnetSensor);
    delete (_headTracker);
    delete (_device);
    delete (_monocularEye);
    delete (_leftEye);
    delete (_rightEye);
    delete (_distortionRenderer);
    delete (_renderContext);
}

#pragma mark - Settings

- (void)orientationDidChange:(NSNotification *)notification {
    _headTracker->updateDeviceOrientation([UIApplication sharedApplication].statusBarOrientation);
}

- (BOOL)vignetteEnabled {
    return _distortionRenderer->isVignetteEnabled();
}

- (void)setVignetteEnabled:(BOOL)vignetteEnabled {
    _distortionRenderer->setVignetteEnabled(vignetteEnabled);
}

- (BOOL)chromaticAberrationCorrectionEnabled {
    return _distortionRenderer->isChromaticAberrationEnabled();
}

- (void)setChromaticAberrationCorrectionEnabled:(BOOL)chromaticAberrationCorrectionEnabled {
    _distortionRenderer->setChromaticAberrationEnabled(chromaticAberrationCorrectionEnabled);
}

- (void)magneticTriggerPressed:(NSNotification *)notification {
    if ([self.renderDelegate respondsToSelector:@selector(magneticTriggerPressed)]) {
        [self.renderDelegate magneticTriggerPressed];
    }
}

- (float)virtualEyeToScreenDistance {
    return _device->getScreenToLensDistance();
}

#pragma mark - Rendering

// Called whenever view changes orientation or layout is changed
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    //[self _reshape];
}

// Called whenever the view needs to render
- (void)drawInMTKView:(nonnull MTKView *)view {
    if (!_rendererInitialized) {
        [self.renderDelegate setupRendererWithView:self context:_renderContext];
        _rendererInitialized = YES;
    }
    
    if (!_headTracker->isReady()) {
        return;
    }
    
    dispatch_semaphore_wait(_inflight_semaphore, DISPATCH_TIME_FOREVER);
    
    @autoreleasepool {
        [self calculateFrameParametersWithLeftEye:_leftEye
                                         rightEye:_rightEye
                                     monocularEye:_monocularEye];
        _frameParamentersReady = YES;
        
        /*
         A single command buffer collects all render events for a frame.
         */
        id <MTLCommandBuffer> commandBuffer = [_renderContext->getCommandQueue() commandBuffer];
        commandBuffer.label = @"CommandBuffer";
        
        /*
         When the command buffer is executed by the GPU, signal the semaphor
         (required by the view since it will signal set up the next buffer).
         */
        __block dispatch_semaphore_t block_sema = _inflight_semaphore;
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            dispatch_semaphore_signal(block_sema);
        }];
        
        VROTransaction::beginImplicitAnimation();
        VROTransaction::update();
        
        if (view.currentRenderPassDescriptor) {
            if (self.vrModeEnabled) {
                if (_distortionCorrectionEnabled) {
                    [self renderVRDistortionInView:view withCommandBuffer:commandBuffer];
                }
                else {
                    [self renderVRInView:view withCommandBuffer:commandBuffer];
                }
            }
            else {
                [self renderMonocularInView:view withCommandBuffer:commandBuffer];
            }
            
            [commandBuffer presentDrawable:view.currentDrawable];
        }
        
        [commandBuffer commit];
        VROTransaction::commitAll();
    }
}

- (void)renderVRDistortionInView:(MTKView *)view withCommandBuffer:(id <MTLCommandBuffer>)commandBuffer {
    _distortionRenderer->updateDistortion(_renderContext->getDevice(), _renderContext->getLibrary(), view);
    
    std::shared_ptr<VRORenderTarget> eyeTarget = _distortionRenderer->bindEyeRenderTarget(commandBuffer);
    _renderContext->setRenderTarget(eyeTarget);
    
    id <MTLRenderCommandEncoder> eyeRenderEncoder = eyeTarget->getRenderEncoder();
    
    [self drawFrameWithLeftEye:_leftEye rightEye:_rightEye];
    [eyeRenderEncoder endEncoding];
    
    std::shared_ptr<VRORenderTarget> screenTarget = std::make_shared<VRORenderTarget>(view, commandBuffer);
    _renderContext->setRenderTarget(screenTarget);
    
    id <MTLRenderCommandEncoder> screenRenderEncoder = screenTarget->getRenderEncoder();
    
    _distortionRenderer->renderEyesToScreen(screenRenderEncoder);
    [screenRenderEncoder endEncoding];
}

- (void)renderVRInView:(MTKView *)view withCommandBuffer:(id <MTLCommandBuffer>)commandBuffer {
    std::shared_ptr<VRORenderTarget> screenTarget = std::make_shared<VRORenderTarget>(view, commandBuffer);
    _renderContext->setRenderTarget(screenTarget);
    
    [self drawFrameWithLeftEye:_leftEye rightEye:_rightEye];
    [screenTarget->getRenderEncoder() endEncoding];
}

- (void)renderMonocularInView:(MTKView *)view withCommandBuffer:(id <MTLCommandBuffer>)commandBuffer {
    std::shared_ptr<VRORenderTarget> screenTarget = std::make_shared<VRORenderTarget>(view, commandBuffer);
    _renderContext->setRenderTarget(screenTarget);
    
    [self drawFrameWithLeftEye:_monocularEye rightEye:nullptr];
    [screenTarget->getRenderEncoder() endEncoding];
}

#pragma mark - View Computation

- (void)calculateFrameParametersWithLeftEye:(VROEye *)leftEye
                                   rightEye:(VROEye *)rightEye
                               monocularEye:(VROEye *)monocularEye {
    
    VROMatrix4f headRotation = _headTracker->getHeadRotation();
    
    float halfLensDistance = _device->getInterLensDistance() * 0.5f;
    VROMatrix4f yFlip = matrix_from_scale(1.0, -1.0, 1.0);
    
    if (self.vrModeEnabled) {
        /*
         The full eye transform is as follows:
         
         1. Set the camera at the origin, looking in the Z negative direction.
         2. Rotate by the camera by the head rotation picked up by the sensors.
         3. Translate the camera by the interlens distance in each direction to get the two eyes.
         */
        VROMatrix4f camera = matrix_float4x4_from_GL(GLKMatrix4MakeLookAt(0, 0, 0,
                                                                          0, 0, -1.0,
                                                                          0, 1.0, 0));
        VROMatrix4f cameraRotated = headRotation.multiply(camera);
        VROMatrix4f leftEyeView  = matrix_from_translation( halfLensDistance, 0, 0).multiply(cameraRotated);
        VROMatrix4f rightEyeView = matrix_from_translation(-halfLensDistance, 0, 0).multiply(cameraRotated);
        
        leftEye->setEyeView(leftEyeView);
        rightEye->setEyeView(rightEyeView);
    }
    else {
        monocularEye->setEyeView(headRotation);
    }
    
    if (_projectionChanged) {
        const VROScreen &screen = _device->getScreen();
        monocularEye->setViewport(0, 0, screen.getWidth(), screen.getHeight());
        
        if (!self.vrModeEnabled) {
            [self updateMonocularEye:monocularEye];
        }
        else if (_distortionCorrectionEnabled) {
            [self updateLeftEye:leftEye rightEye:rightEye];
            _distortionRenderer->fovDidChange(leftEye->getFOV(), rightEye->getFOV(),
                                              [self virtualEyeToScreenDistance]);
        }
        else {
            [self updateUndistortedFOVAndViewport];
        }
        
        _projectionChanged = NO;
    }
    
    if (self.distortionCorrectionEnabled && _distortionRenderer->viewportsChanged()) {
        _distortionRenderer->updateViewports(leftEye, rightEye);
    }
}

- (void)updateMonocularEye:(VROEye *)monocularEye {
    const VROScreen &screen = _device->getScreen();
    const float monocularBottomFov = 22.5f;
    const float monocularLeftFov = GLKMathRadiansToDegrees(
                                                           atanf(
                                                                 tanf(GLKMathDegreesToRadians(monocularBottomFov))
                                                                 * screen.getWidthInMeters()
                                                                 / screen.getHeightInMeters()));
    monocularEye->setFOV(monocularLeftFov, monocularLeftFov, monocularBottomFov, monocularBottomFov);
}

- (void)updateLeftEye:(VROEye *)leftEye rightEye:(VROEye *)rightEye {
    const VROScreen &screen = _device->getScreen();
    
    const VRODistortion &distortion = _device->getDistortion();
    float eyeToScreenDistance = [self virtualEyeToScreenDistance];
    
    float outerDistance = (screen.getWidthInMeters() - _device->getInterLensDistance() ) / 2.0f;
    float innerDistance = _device->getInterLensDistance() / 2.0f;
    float bottomDistance = _device->getVerticalDistanceToLensCenter() - screen.getBorderSizeInMeters();
    float topDistance = screen.getHeightInMeters() + screen.getBorderSizeInMeters() - _device->getVerticalDistanceToLensCenter();
    
    float outerAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(outerDistance / eyeToScreenDistance)));
    float innerAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(innerDistance / eyeToScreenDistance)));
    float bottomAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(bottomDistance / eyeToScreenDistance)));
    float topAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(topDistance / eyeToScreenDistance)));
    
    leftEye->setFOV(MIN(outerAngle,  _device->getMaximumLeftEyeFOV().getLeft()),
                    MIN(innerAngle,  _device->getMaximumLeftEyeFOV().getRight()),
                    MIN(bottomAngle, _device->getMaximumLeftEyeFOV().getBottom()),
                    MIN(topAngle,    _device->getMaximumLeftEyeFOV().getTop()));
    
    const VROFieldOfView &leftEyeFov = leftEye->getFOV();
    rightEye->setFOV(leftEyeFov.getRight(),
                     leftEyeFov.getLeft(),
                     leftEyeFov.getBottom(),
                     leftEyeFov.getTop());
}

- (void)updateUndistortedFOVAndViewport {
    const VROScreen &screen = _device->getScreen();
    
    float halfInterLensDistance = _device->getInterLensDistance() * 0.5f;
    float eyeToScreenDistance = [self virtualEyeToScreenDistance];
    
    float left = screen.getWidthInMeters() / 2.0f - halfInterLensDistance;
    float right = halfInterLensDistance;
    float bottom = _device->getVerticalDistanceToLensCenter() - screen.getBorderSizeInMeters();
    float top = screen.getBorderSizeInMeters() + screen.getHeightInMeters() - _device->getVerticalDistanceToLensCenter();
    
    _leftEye->setFOV(GLKMathRadiansToDegrees(atan2f(left, eyeToScreenDistance)),
                     GLKMathRadiansToDegrees(atan2f(right, eyeToScreenDistance)),
                     GLKMathRadiansToDegrees(atan2f(bottom, eyeToScreenDistance)),
                     GLKMathRadiansToDegrees(atan2f(top, eyeToScreenDistance)));
    
    const VROFieldOfView &leftEyeFov = _leftEye->getFOV();
    _rightEye->setFOV(leftEyeFov.getRight(),
                      leftEyeFov.getLeft(),
                      leftEyeFov.getBottom(),
                      leftEyeFov.getTop());
    
    _leftEye->setViewport(0, 0, screen.getWidth() / 2, screen.getHeight());
    _rightEye->setViewport(screen.getWidth() / 2, 0, screen.getWidth() / 2, screen.getHeight());
}

- (void)layoutSubviews {
    [super layoutSubviews];
    [self updateRenderViewSize:self.bounds.size];
}

#pragma mark - Stereo renderer methods

- (void)updateRenderViewSize:(CGSize)size {
    if (self.vrModeEnabled) {
        [self.renderDelegate renderViewDidChangeSize:CGSizeMake(size.width / 2, size.height) context:_renderContext];
    }
    else {
        [self.renderDelegate renderViewDidChangeSize:CGSizeMake(size.width, size.height) context:_renderContext];
    }
}

- (void)drawFrameWithLeftEye:(VROEye *)leftEye
                    rightEye:(VROEye *)rightEye {
    
    id <MTLRenderCommandEncoder> renderEncoder = _renderContext->getRenderTarget()->getRenderEncoder();
    
    const float zNear = 0.1;
    const float zFar  = 100;
    
    _renderContext->notifyFrameStart();
    
    [renderEncoder setViewport:leftEye->getViewport().toMetalViewport()];
    [renderEncoder setScissorRect:leftEye->getViewport().toMetalScissor()];
    
    _renderContext->setViewMatrix(leftEye->getEyeView());
    _renderContext->setProjectionMatrix(leftEye->perspective(zNear, zFar));
    
    [self.renderDelegate renderEye:VROEyeTypeLeft context:_renderContext];
    
    if (rightEye == nullptr) {
        return;
    }
    
    [renderEncoder setViewport:rightEye->getViewport().toMetalViewport()];
    [renderEncoder setScissorRect:rightEye->getViewport().toMetalScissor()];
    
    _renderContext->setViewMatrix(rightEye->getEyeView());
    _renderContext->setProjectionMatrix(rightEye->perspective(zNear, zFar));
    
    [self.renderDelegate renderEye:VROEyeTypeRight context:_renderContext];
    _renderContext->notifyFrameEnd();
}

- (VRORenderContext *)renderContext {
    return _renderContext;
}

@end
