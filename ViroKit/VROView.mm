//
//  VROView.m
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import "VROView.h"
#import "VROTime.h"
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
#import "VROProjector.h"
#import "VROAllocationTracker.h"
#import "VROScene.h"
#import "VROSceneController.h"
#import "VROLog.h"
#import "VROCameraMutable.h"

static const float kDefaultSceneTransitionDuration = 1.0;

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
    
    BOOL _projectionChanged;
    BOOL _frameParamentersReady;
    BOOL _rendererInitialized;
    
    /*
     Internal representation of the camera.
     */
    std::shared_ptr<VROCameraMutable> _camera;
    
    /*
     Scene transition variables.
     */
    float _sceneTransitionDuration;
    float _sceneTransitionStartTime;
    std::unique_ptr<VROTimingFunction> _sceneTransitionTimingFunction;

}

@property (readwrite, nonatomic) VROSceneController *outgoingSceneController;

@end

@implementation VROView

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
    // Do not allow the display to go into sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    _magnetSensor = new VROMagnetSensor();
    _headTracker = new VROHeadTracker();
    _device = new VRODevice([UIScreen mainScreen]);
    _camera = std::make_shared<VROCameraMutable>();
    
    _monocularEye = new VROEye(VROEyeType::Monocular);
    _leftEye = new VROEye(VROEyeType::Left);
    _rightEye = new VROEye(VROEyeType::Right);
    
    _distortionRenderer = new VRODistortionRenderer(*_device);
    _vrModeEnabled = YES;
    _rendererInitialized = NO;
    
    _projectionChanged = YES;
    _frameParamentersReady = NO;
    
    _headTracker->startTracking([UIApplication sharedApplication].statusBarOrientation);
    _magnetSensor->start();
        
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
    
    _HUD = [[VROScreenUIView alloc] init];
    
    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap:)];
    [self addGestureRecognizer:tapRecognizer];
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

- (float)virtualEyeToScreenDistance {
    return _device->getScreenToLensDistance();
}

#pragma mark - Camera

- (void)setPosition:(VROVector3f)position {
    _camera->setPosition(position);
}

- (void)setBaseRotation:(VROQuaternion)rotation {
    _camera->setBaseRotation(rotation);
}

- (float)worldPerScreenAtDepth:(float)distance {
    /*
     Arbitrarily chose left eye's left FOV. tan(fov) = perp/distance, where
     perp is in the direction perpendicular to the camera's up vector and
     forward vector, and distance is in the direction of the camera's forward
     vector.
     */
    float radians = _leftEye->getFOV().getLeft();
    float perp = distance * tan(radians);
    
    /*
     The perspective divide is perp divided by half the viewport.
     */
    return perp / (_leftEye->getViewport().getWidth() / 2.0f);
}

#pragma mark - Rendering

// Called whenever view changes orientation or layout is changed
- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size {
    //[self _reshape];
}

// Called whenever the view needs to render
- (void)drawInMTKView:(nonnull MTKView *)view {
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
                [self renderVRDistortionInView:view withCommandBuffer:commandBuffer];
            }
            else {
                [self renderMonocularInView:view withCommandBuffer:commandBuffer];
            }
            
            [commandBuffer presentDrawable:view.currentDrawable];
        }
        
        [commandBuffer commit];
        VROTransaction::commitAll();
        
        _renderContext->incFrame();
    }
    
    ALLOCATION_TRACKER_PRINT();
}

- (void)renderVRDistortionInView:(MTKView *)view withCommandBuffer:(id <MTLCommandBuffer>)commandBuffer {
    _distortionRenderer->updateDistortion(_renderContext->getDevice(), _renderContext->getLibrary(), view);
    
    std::shared_ptr<VRORenderTarget> eyeTarget = _distortionRenderer->bindEyeRenderTarget(commandBuffer);
    _renderContext->setRenderTarget(eyeTarget);
    
    if (!_rendererInitialized) {
        [self.renderDelegate setupRendererWithView:self context:_renderContext];
        _rendererInitialized = YES;
    }
    
    id <MTLRenderCommandEncoder> eyeRenderEncoder = eyeTarget->getRenderEncoder();
    
    [self drawFrameWithLeftEye:_leftEye rightEye:_rightEye];
    [eyeRenderEncoder endEncoding];
    
    std::shared_ptr<VRORenderTarget> screenTarget = std::make_shared<VRORenderTarget>(view, commandBuffer);
    _renderContext->setRenderTarget(screenTarget);
    
    id <MTLRenderCommandEncoder> screenRenderEncoder = screenTarget->getRenderEncoder();
    
    _distortionRenderer->renderEyesToScreen(screenRenderEncoder, _renderContext->getFrame());
    [screenRenderEncoder endEncoding];
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
    
    VROVector3f cameraForward(0, 0, -1.0);

    VROMatrix4f headRotation = _headTracker->getHeadRotation();
    
    VROCamera camera;
    camera.setHeadRotation(headRotation.invert());
    camera.setBaseRotation(_camera->getBaseRotation());
    camera.setPosition(_camera->getPosition());
    
    _renderContext->setCamera(camera);
    
    float halfLensDistance = _device->getInterLensDistance() * 0.5f;
    if (self.vrModeEnabled) {
        /*
         The full eye transform is as follows:
         
         1. Set the camera at the origin, looking in the Z negative direction.
         2. Rotate by the camera by the head rotation picked up by the sensors.
         3. Translate the camera by the interlens distance in each direction to get the two eyes.
         */
        VROMatrix4f cameraMatrix = camera.computeLookAtMatrix();
       
        leftEye->setEyeView(matrix_from_translation( halfLensDistance, 0, 0).multiply(cameraMatrix));
        rightEye->setEyeView(matrix_from_translation(-halfLensDistance, 0, 0).multiply(cameraMatrix));
        
        /*
         In VR mode, the monocular eye holds the non-stereoscopic matrix (which is used for objects that
         should appear distant, like skyboxes.
         */
        monocularEye->setEyeView(cameraMatrix);
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
        else {
            [self updateLeftEye:leftEye rightEye:rightEye];
            _distortionRenderer->fovDidChange(leftEye->getFOV(), rightEye->getFOV(),
                                              [self virtualEyeToScreenDistance]);
        }
        
        _projectionChanged = NO;
    }
    
    if (_distortionRenderer->viewportsChanged()) {
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
    
    BOOL sceneTransitionActive = [self processSceneTransition];
    _renderContext->notifyFrameStart();
    
    [renderEncoder setViewport:leftEye->getViewport().toMetalViewport()];
    [renderEncoder setScissorRect:leftEye->getViewport().toMetalScissor()];
    
    _renderContext->setMonocularViewMatrix(_monocularEye->getEyeView());
    _renderContext->setHUDViewMatrix(matrix_from_translation(_device->getInterLensDistance() * 0.5, 0, 0).multiply(leftEye->getEyeView().invert()));
    _renderContext->setViewMatrix(leftEye->getEyeView());
    _renderContext->setProjectionMatrix(leftEye->perspective(zNear, zFar));
    _renderContext->setEyeType(VROEyeType::Left);
    
    [self renderEye:VROEyeType::Left];
    [_HUD updateWithContext:_renderContext];
    [_HUD renderEye:leftEye withContext:_renderContext];
    
    if (rightEye == nullptr) {
        _renderContext->notifyFrameEnd();
        return;
    }
    
    [renderEncoder setViewport:rightEye->getViewport().toMetalViewport()];
    [renderEncoder setScissorRect:rightEye->getViewport().toMetalScissor()];
    
    _renderContext->setHUDViewMatrix(matrix_from_translation(-_device->getInterLensDistance() * 0.5, 0, 0).multiply(rightEye->getEyeView().invert()));
    _renderContext->setViewMatrix(rightEye->getEyeView());
    _renderContext->setProjectionMatrix(rightEye->perspective(zNear, zFar));
    _renderContext->setEyeType(VROEyeType::Right);
    
    [self renderEye:VROEyeType::Right];
    [_HUD renderEye:rightEye withContext:_renderContext];
    
    _renderContext->notifyFrameEnd();
    
    if (!sceneTransitionActive) {
        self.outgoingSceneController = nil;
    }
}

- (void)renderEye:(VROEyeType)eyeType {
    [self.renderDelegate willRenderEye:eyeType context:_renderContext];
    if (_sceneController) {
        if (_outgoingSceneController) {
            [_outgoingSceneController sceneWillRender:_renderContext];
            [_sceneController sceneWillRender:_renderContext];
            
            _outgoingSceneController.scene->renderBackground(*_renderContext);
            _sceneController.scene->renderBackground(*_renderContext);
            
            _outgoingSceneController.scene->render(*_renderContext);
            _sceneController.scene->render(*_renderContext);
        }
        else {
            [_sceneController sceneWillRender:_renderContext];
            _sceneController.scene->renderBackground(*_renderContext);
            _sceneController.scene->render(*_renderContext);
        }
    }
    [self.renderDelegate didRenderEye:eyeType context:_renderContext];
}

- (VRORenderContext *)renderContext {
    return _renderContext;
}

#pragma mark - Reticle

- (void)handleTap:(UIGestureRecognizer *)gestureRecognizer {
    [_HUD.reticle trigger];
    [self.renderDelegate reticleTapped:_renderContext->getCamera().getForward()
                               context:_renderContext];
    if (self.sceneController) {
        [self.sceneController reticleTapped:_renderContext->getCamera().getForward()
                                    context:_renderContext];
    }
}

#pragma mark - Scene Loading

- (void)setSceneController:(VROSceneController *)sceneController {
    VROSceneController *outgoingSceneController = _sceneController;
    
    [sceneController sceneWillAppear:self.renderContext];
    if (outgoingSceneController) {
        [outgoingSceneController sceneWillDisappear:self.renderContext];
    }
    
    _sceneController = sceneController;
    
    [sceneController sceneDidAppear:self.renderContext];
    if (outgoingSceneController) {
        [outgoingSceneController sceneDidDisappear:self.renderContext];
    }
}

- (void)setSceneController:(VROSceneController *)sceneController animated:(BOOL)animated {
    if (!animated || !self.sceneController) {
        self.sceneController = sceneController;
        return;
    }
    
    [self setSceneController:sceneController duration:kDefaultSceneTransitionDuration
              timingFunction:VROTimingFunctionType::EaseIn];
}

- (void)setSceneController:(VROSceneController *)sceneController duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    
    _outgoingSceneController = _sceneController;
    _sceneController = sceneController;
    
    _sceneTransitionStartTime = VROTimeCurrentSeconds();
    _sceneTransitionDuration = seconds;
    _sceneTransitionTimingFunction = VROTimingFunction::forType(timingFunctionType);
    
    [self.sceneController sceneWillAppear:self.renderContext];
    [self.outgoingSceneController sceneWillDisappear:self.renderContext];
    
    [self.sceneController startIncomingTransition:self.renderContext duration:seconds];
    [self.outgoingSceneController startOutgoingTransition:self.renderContext duration:seconds];
}

- (BOOL)processSceneTransition {
    if (!self.sceneController || !self.outgoingSceneController) {
        return NO;
    }
    
    float percent = (VROTimeCurrentSeconds() - _sceneTransitionStartTime) / _sceneTransitionDuration;
    float t = _sceneTransitionTimingFunction->getT(percent);
    
    BOOL sceneTransitionActive = percent < 0.9999;
    if (sceneTransitionActive) {
        [self.sceneController animateIncomingTransition:self.renderContext percentComplete:t];
        [self.outgoingSceneController animateOutgoingTransition:self.renderContext percentComplete:t];
    }
    else {
        [self.sceneController animateIncomingTransition:self.renderContext percentComplete:1.0];
        [self.outgoingSceneController animateOutgoingTransition:self.renderContext percentComplete:1.0];
        
        [self.sceneController endIncomingTransition:self.renderContext];
        [self.outgoingSceneController endOutgoingTransition:self.renderContext];
        
        [self.sceneController sceneDidAppear:self.renderContext];
        [self.outgoingSceneController sceneDidDisappear:self.renderContext];
    }
    
    return sceneTransitionActive;
}

@end
