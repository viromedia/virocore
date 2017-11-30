//
//  VROViewMetal.m
//  ViroRenderer
//
//  Created by Raj Advani on 4/22/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROViewMetal.h"
#if VRO_METAL

#import "VRODriverMetal.h"
#import "VROAllocationTracker.h"
#import "VRORenderer.h"
#import "VROHeadTracker.h"
#import "VRODevice.h"
#import "VROScreen.h"
#import "VRODistortionRenderer.h"
#import "VROEye.h"
#import "VRORenderDelegateiOS.h"
#import "VROSceneController.h"
#import "VROSceneControlleriOS.h"

@interface VROViewMetal () {
    
    int _frameNumber;
    bool _vrModeEnabled;
    bool _projectionChanged;
    
    VROEye *_monocularEye;
    VROEye *_leftEye;
    VROEye *_rightEye;
    
    std::shared_ptr<VRORenderer> _renderer;
    
    VROHeadTracker *_headTracker;
    std::shared_ptr<VRODevice> _vrDevice;
    std::shared_ptr<VRODriverMetal> _driver;
    
    VRODistortionRenderer *_distortionRenderer;
    dispatch_semaphore_t _inflight_semaphore;
    
    VROSceneController *_sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    
}

@property (readwrite, nonatomic) std::shared_ptr<VRORenderer> renderer;

@end

@implementation VROViewMetal

@dynamic renderDelegate;

#pragma mark - Initialization

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
    _frameNumber = 0;
    _vrModeEnabled = true;
    _projectionChanged = true;
    _vrDevice = std::make_shared<VRODevice>([UIScreen mainScreen]);
    
    self.device = MTLCreateSystemDefaultDevice();
    self.delegate = self;
    self.depthStencilPixelFormat = MTLPixelFormatDepth32Float_Stencil8;

    _driver = std::make_shared<VRODriverMetal>(self.device);
    _distortionRenderer = new VRODistortionRenderer(_vrDevice);
    _inflight_semaphore = dispatch_semaphore_create(3);
        
    _monocularEye = new VROEye(VROEyeType::Monocular);
    _leftEye = new VROEye(VROEyeType::Left);
    _rightEye = new VROEye(VROEyeType::Right);
        
    _headTracker = new VROHeadTracker();
    _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
    
    // Do not allow the display to go into sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    self.renderer = std::make_shared<VRORenderer>();
    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self
                                                                                    action:@selector(handleTap:)];
    [self addGestureRecognizer:tapRecognizer];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    
    delete (_distortionRenderer);
    delete (_headTracker);
    delete (_monocularEye);
    delete (_leftEye);
    delete (_rightEye);
}

#pragma mark - Settings

- (void)orientationDidChange:(NSNotification *)notification {
    _headTracker->updateDeviceOrientation([UIApplication sharedApplication].statusBarOrientation);
}

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    _renderDelegateWrapper = std::make_shared<VRORenderDelegateiOS>(renderDelegate);
    self.renderer->setDelegate(_renderDelegateWrapper);
}

#pragma mark - Camera

- (void)setPointOfView:(std::shared_ptr<VRONode>)node {
    self.renderer->setPointOfView(node);
}

- (VROEye *)eyeForType:(VROEyeType)type {
    switch (type) {
        case VROEyeType::Left:
            return _leftEye;
        case VROEyeType::Right:
            return _rightEye;
        default:
            return _monocularEye;
    }
}

#pragma mark - Reticle

- (void)handleTap:(UIGestureRecognizer *)gestureRecognizer {
    _renderer->handleTap();
}

- (std::shared_ptr<VROReticle>)reticle {
    return _renderer->getReticle();
}

#pragma mark - Scene Loading

- (VROSceneController *)sceneController {
    return _sceneController;
}

- (void)setSceneController:(VROSceneController *)sceneController {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController.internal, *_driver);
}

- (void)setSceneController:(VROSceneController *)sceneController duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController.internal, seconds, timingFunctionType, *_driver);
}

#pragma mark - Frame Listeners

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
}

#pragma mark - Rendering

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    
}

- (void)drawInMTKView:(nonnull MTKView *)view {
    if (!_headTracker->isReady()) {
        return;
    }
    
    @autoreleasepool {
        dispatch_semaphore_wait(_inflight_semaphore, DISPATCH_TIME_FOREVER);
        
        /*
         A single command buffer collects all render events for a frame.
         */
        VRODriverMetal *driver = (VRODriverMetal *)_driver.get();
        
        id <MTLCommandBuffer> commandBuffer = [driver->getCommandQueue() commandBuffer];
        commandBuffer.label = @"CommandBuffer";
        
        /*
         When the command buffer is executed by the GPU, signal the semaphor
         (required by the view since it will signal set up the next buffer).
         */
        __block dispatch_semaphore_t block_sema = _inflight_semaphore;
        [commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
            dispatch_semaphore_signal(block_sema);
        }];
        
        [self calculateFrameParameters];
        [self renderVRDistortionWithCommandBuffer:commandBuffer];
        
        [commandBuffer presentDrawable:self.currentDrawable];
        [commandBuffer commit];
    }
    
    ++_frameNumber;
    ALLOCATION_TRACKER_PRINT();
}

- (void)renderVRDistortionWithCommandBuffer:(id <MTLCommandBuffer>)commandBuffer {
    VRODriverMetal *driver = (VRODriverMetal *)_driver.get();
    _distortionRenderer->updateDistortion(driver->getDevice(), driver->getLibrary(), self);
    
    std::shared_ptr<VRORenderTarget> eyeTarget = _distortionRenderer->bindEyeRenderTarget(commandBuffer);
    driver->setRenderTarget(eyeTarget);
    
    VROMatrix4f headRotation = _headTracker->getHeadRotation();
    _renderer->prepareFrame(_frameNumber, _leftEye->getViewport(), _leftEye->getFOV(),
                            headRotation.invert(), *driver);
    
    float halfLensDistance = _vrDevice->getInterLensDistance() * 0.5f;
    VROMatrix4f leftEyeMatrix  = matrix_from_translation( halfLensDistance, 0, 0);
    VROMatrix4f rightEyeMatrix = matrix_from_translation(-halfLensDistance, 0, 0);
    
    id <MTLRenderCommandEncoder> eyeRenderEncoder = eyeTarget->getRenderEncoder();
    [eyeRenderEncoder setViewport:_leftEye->getViewport().toMetalViewport()];
    [eyeRenderEncoder setScissorRect:_leftEye->getViewport().toMetalScissor()];
    
    _renderer->renderEye(_leftEye->getType(), leftEyeMatrix, _leftEye->getPerspectiveMatrix(),
                         *driver);
    
    [eyeRenderEncoder setViewport:_rightEye->getViewport().toMetalViewport()];
    [eyeRenderEncoder setScissorRect:_rightEye->getViewport().toMetalScissor()];
    
    _renderer->renderEye(_rightEye->getType(), rightEyeMatrix, _rightEye->getPerspectiveMatrix(),
                         *driver);
    _renderer->endFrame(*driver);
    
    [eyeRenderEncoder endEncoding];
    
    std::shared_ptr<VRORenderTarget> screenTarget = std::make_shared<VRORenderTarget>(self, commandBuffer);
    id <MTLRenderCommandEncoder> screenRenderEncoder = screenTarget->getRenderEncoder();
    
    _distortionRenderer->renderEyesToScreen(screenRenderEncoder, _frameNumber);
    [screenRenderEncoder endEncoding];
}

#pragma mark - View Computation

- (void)calculateFrameParameters {
    if (_projectionChanged) {
        const VROScreen &screen = _vrDevice->getScreen();
        _monocularEye->setViewport(0, 0, screen.getWidth(), screen.getHeight());
        
        if (!_vrModeEnabled) {
            [self updateMonocularEye];
        }
        else {
            [self updateLeftRightEyes];
        }
        
        _projectionChanged = NO;
    }
}

- (void)updateMonocularEye {
    const VROScreen &screen = _vrDevice->getScreen();
    const float monocularBottomFov = 22.5f;
    const float monocularLeftFov = GLKMathRadiansToDegrees(
                                                           atanf(
                                                                 tanf(GLKMathDegreesToRadians(monocularBottomFov))
                                                                 * screen.getWidthInMeters()
                                                                 / screen.getHeightInMeters()));
    _monocularEye->setFOV(monocularLeftFov, monocularLeftFov, monocularBottomFov, monocularBottomFov,
                          kZNear, kZFar);
}

- (void)updateLeftRightEyes {
    const VROScreen &screen = _vrDevice->getScreen();
    
    const VRODistortion &distortion = _vrDevice->getDistortion();
    float eyeToScreenDistance = _vrDevice->getScreenToLensDistance();
    
    float outerDistance = (screen.getWidthInMeters() - _vrDevice->getInterLensDistance() ) / 2.0f;
    float innerDistance = _vrDevice->getInterLensDistance() / 2.0f;
    float bottomDistance = _vrDevice->getVerticalDistanceToLensCenter() - screen.getBorderSizeInMeters();
    float topDistance = screen.getHeightInMeters() + screen.getBorderSizeInMeters() - _vrDevice->getVerticalDistanceToLensCenter();
    
    float outerAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(outerDistance / eyeToScreenDistance)));
    float innerAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(innerDistance / eyeToScreenDistance)));
    float bottomAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(bottomDistance / eyeToScreenDistance)));
    float topAngle = GLKMathRadiansToDegrees(atanf(distortion.distort(topDistance / eyeToScreenDistance)));
    
    _leftEye->setFOV(MIN(outerAngle,  _vrDevice->getMaximumLeftEyeFOV().getLeft()),
                     MIN(innerAngle,  _vrDevice->getMaximumLeftEyeFOV().getRight()),
                     MIN(bottomAngle, _vrDevice->getMaximumLeftEyeFOV().getBottom()),
                     MIN(topAngle,    _vrDevice->getMaximumLeftEyeFOV().getTop()),
                     kZNear, kZFar);
    
    const VROFieldOfView &leftEyeFov = _leftEye->getFOV();
    _rightEye->setFOV(leftEyeFov.getRight(),
                      leftEyeFov.getLeft(),
                      leftEyeFov.getBottom(),
                      leftEyeFov.getTop(),
                      kZNear, kZFar);
    
    _distortionRenderer->fovDidChange(_leftEye, _rightEye,
                                      _vrDevice->getScreenToLensDistance());
}

@end

#endif
