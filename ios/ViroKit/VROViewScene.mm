//
//  VROViewScene.m
//  ViroRenderer
//
//  Created by Raj Advani on 3/1/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#import "VROViewScene.h"
#import "VRORenderer.h"
#import "VROSceneController.h"
#import "VRORenderDelegateiOS.h"
#import "VROTime.h"
#import "VROEye.h"
#import "VRODriverOpenGLiOS.h"
#import "VROApiKeyValidator.h"
#import "VROApiKeyValidatorDynamo.h"
#import "VROConvert.h"
#import "VRONodeCamera.h"
#import "vr/gvr/capi/include/gvr_audio.h"
#import "VROChoreographer.h"
#import "VROInputControllerAR.h"
#import "VROWeakProxy.h"
#import "VROViewRecorder.h"

static VROVector3f const kZeroVector = VROVector3f();

@interface VROViewScene () {
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<VROInputControllerAR> _inputController;
    
    CADisplayLink *_displayLink;
    int _frame;
    double _suspendedNotificationTime;
}

@property (readwrite, nonatomic) id <VROApiKeyValidator> keyValidator;
@property (readwrite, nonatomic) VROViewRecorder *viewRecorder;

@end

@implementation VROViewScene

@dynamic renderDelegate;
@dynamic sceneController;

#pragma mark - Initialization

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        _suspended = YES;
        VRORendererConfiguration config;
        [self initRenderer:config];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame
                       config:(VRORendererConfiguration)config
                      context:(EAGLContext *)context {
    self = [super initWithFrame:frame context:context];
    if (self) {
        _suspended = YES;
        [self initRenderer:config];
    }
    return self;
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    if (_inputController) {
        _inputController->setViewportSize(self.frame.size.width * self.contentScaleFactor,
                                          self.frame.size.height * self.contentScaleFactor);
    }
}

- (void)initRenderer:(VRORendererConfiguration)config {
    VROPlatformSetType(VROPlatformType::iOSSceneView);
    if (!self.context) {
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        self.context = context;
    }
    VROThreadRestricted::setThread(VROThreadName::Renderer);
    
    /*
     Setup the GLKView.
     */
    self.enableSetNeedsDisplay = NO;
    self.drawableColorFormat = GLKViewDrawableColorFormatSRGBA8888;
    self.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    self.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    self.drawableMultisample = GLKViewDrawableMultisample4X;
    
    [EAGLContext setCurrentContext:self.context];
    
    /*
     Setup the animation loop for the GLKView.
     */
    VROWeakProxy *proxy = [VROWeakProxy weakProxyForObject:self];
    _displayLink = [CADisplayLink displayLinkWithTarget:proxy selector:@selector(display)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

    /*
     Disable going to sleep, and setup notifications.
     */
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationWillResignActive:)
                                                 name:UIApplicationWillResignActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(applicationDidBecomeActive:)
                                                 name:UIApplicationDidBecomeActiveNotification
                                               object:nil];
    
    /*
     Create Viro renderer objects.
     */
    _driver = std::make_shared<VRODriverOpenGLiOS>(self, self.context);
    _frame = 0;
    _suspendedNotificationTime = VROTimeCurrentSeconds();
    _inputController = std::make_shared<VROInputControllerAR>(self.frame.size.width * self.contentScaleFactor,
                                                              self.frame.size.height * self.contentScaleFactor);
    _renderer = std::make_shared<VRORenderer>(config, _inputController);
    
    /*
     Set up the Audio Session properly for recording and playing back audio. We need
     to do this *AFTER* we init _gvrAudio (in driver construction), because it resets
     some setting, else audio recording won't work.
     */
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayAndRecord
             withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker
                   error:nil];
    
    self.keyValidator = [[VROApiKeyValidatorDynamo alloc] init];

    UIRotationGestureRecognizer *rotateGesture = [[UIRotationGestureRecognizer alloc] initWithTarget:self action:@selector(handleRotate:)];
    [rotateGesture setDelegate:self];
    [self addGestureRecognizer:rotateGesture];

    UIPinchGestureRecognizer *pinchGesture = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(handlePinch:)];
    [rotateGesture setDelegate:self];
    [self addGestureRecognizer:pinchGesture];

    /*
     Use a pan gesture instead of a 0 second long press gesture recoginizer because
     it seems to play better with the other two recoginizers
     */
    UIPanGestureRecognizer *panGesture = [[UIPanGestureRecognizer alloc] initWithTarget:self
                                                                                 action:@selector(handleLongPress:)];
    [panGesture setDelegate:self];
    [self addGestureRecognizer:panGesture];

    UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleTap:)];
    [tapGesture setDelegate:self];
    [self addGestureRecognizer:tapGesture];
    
    self.viewRecorder = [[VROViewRecorder alloc] initWithView:self renderer:_renderer driver:_driver];
}

#pragma mark - Gesture handlers

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {
    if ([gestureRecognizer isKindOfClass:[UIPanGestureRecognizer class]] || [otherGestureRecognizer isKindOfClass:[UIPanGestureRecognizer class]]) {
        return NO;
    }
    return YES;
}

- (void)handleRotate:(UIRotationGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:[recognizer.view superview]];
    VROVector3f viewportTouchPos = VROVector3f(location.x * self.contentScaleFactor, location.y * self.contentScaleFactor);
    
    if(recognizer.state == UIGestureRecognizerStateBegan) {
        _inputController->onRotateStart(viewportTouchPos);
    } else if(recognizer.state == UIGestureRecognizerStateChanged) {
        // Note: we need to "negate" the rotation because the value returned is "opposite" of our platform.
        _inputController->onRotate(-recognizer.rotation); // already in radians
    } else if(recognizer.state == UIGestureRecognizerStateEnded) {
        _inputController->onRotateEnd();
    }
}

- (void)handlePinch:(UIPinchGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:[recognizer.view superview]];
    VROVector3f viewportTouchPos = VROVector3f(location.x * self.contentScaleFactor, location.y * self.contentScaleFactor);
  
    if(recognizer.state == UIGestureRecognizerStateBegan) {
        _inputController->onPinchStart(viewportTouchPos);
    } else if(recognizer.state == UIGestureRecognizerStateChanged) {
        _inputController->onPinchScale(recognizer.scale);
    } else if(recognizer.state == UIGestureRecognizerStateEnded) {
        _inputController->onPinchEnd();
    }
}

- (void)handleLongPress:(UIPanGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:[recognizer.view superview]];
    
    VROVector3f viewportTouchPos = VROVector3f(location.x * self.contentScaleFactor, location.y * self.contentScaleFactor);
    
    if (recognizer.state == UIGestureRecognizerStateBegan) {
        _inputController->onScreenTouchDown(viewportTouchPos);
    } else if (recognizer.state == UIGestureRecognizerStateEnded) {
        _inputController->onScreenTouchUp(viewportTouchPos);
    } else {
        _inputController->onScreenTouchMove(viewportTouchPos);
    }
}

- (void)handleTap:(UITapGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:[recognizer.view superview]];
    VROVector3f viewportTouchPos = VROVector3f(location.x * self.contentScaleFactor, location.y * self.contentScaleFactor);
    
    if (recognizer.state == UIGestureRecognizerStateRecognized) {
        _inputController->onScreenTouchDown(viewportTouchPos);
        _inputController->onScreenTouchUp(viewportTouchPos);
    }
}

- (void)dealloc {
    VROThreadRestricted::unsetThread();
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    if (_displayLink) {
        [_displayLink invalidate];
    }
}

- (void)deleteGL {
    [self.viewRecorder deleteGL];
    if (_sceneController) {
        _sceneController->getScene()->getRootNode()->deleteGL();
    }
}

- (void)setPaused:(BOOL)paused {
    [_displayLink setPaused:paused];
}

- (BOOL)setShadowsEnabled:(BOOL)enabled {
    return _renderer->getChoreographer()->setShadowsEnabled(enabled);
}

- (BOOL)setHDREnabled:(BOOL)enabled {
    return _renderer->getChoreographer()->setHDREnabled(enabled);
}

- (BOOL)setPBREnabled:(BOOL)enabled {
    return _renderer->getChoreographer()->setPBREnabled(enabled);
}

- (BOOL)setBloomEnabled:(BOOL)enabled {
    return _renderer->getChoreographer()->setBloomEnabled(enabled);
}

#pragma mark - Recording and Screenshots

- (void)startVideoRecording:(NSString *)fileName
           saveToCameraRoll:(BOOL)saveToCamera
                 errorBlock:(VROViewRecordingErrorBlock)errorBlock {
    [self.viewRecorder startVideoRecording:fileName saveToCameraRoll:saveToCamera errorBlock:errorBlock];
}

- (void)stopVideoRecordingWithHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    [self.viewRecorder stopVideoRecordingWithHandler:completionHandler];
}

- (void)takeScreenshot:(NSString *)fileName
      saveToCameraRoll:(BOOL)saveToCamera
 withCompletionHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    [self.viewRecorder takeScreenshot:fileName saveToCameraRoll:saveToCamera withCompletionHandler:completionHandler];
}

#pragma mark - Settings and Notifications

- (void)orientationDidChange:(NSNotification *)notification {
   
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    _displayLink.paused = YES;
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    _displayLink.paused = NO;
}

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    _renderDelegateWrapper = std::make_shared<VRORenderDelegateiOS>(renderDelegate);
    _renderer->setDelegate(_renderDelegateWrapper);
}

- (void)setVrMode:(BOOL)enabled {
    // No-op in scene view
}

- (void)setDebugHUDEnabled:(BOOL)enabled {
    _renderer->setDebugHUDEnabled(enabled);
}

- (NSString *)getPlatform {
    return @"scene";
}

- (NSString *)getHeadset {
    return [NSString stringWithUTF8String:_inputController->getHeadset().c_str()];
}

- (NSString *)getController {
    return [NSString stringWithUTF8String:_inputController->getController().c_str()];
}

#pragma mark - Key Validation

- (void)validateApiKey:(NSString *)apiKey withCompletionBlock:(VROViewValidApiKeyBlock)completionBlock {
    // If the user gives us a key, then let them use the API until we successfully checked the key.
    self.suspended = NO;
    __weak typeof(self) weakSelf = self;
    
    VROApiKeyValidatorBlock validatorCompletionBlock = ^(BOOL valid) {
        typeof(weakSelf) strongSelf = weakSelf;
        if (!strongSelf) {
            return;
        }
        
        strongSelf.suspended = !valid;
        completionBlock(valid);
        NSLog(@"[ApiKeyValidator] The key is %@!", valid ? @"valid" : @"invalid");
    };
    [self.keyValidator validateApiKey:apiKey platform:[self getPlatform] withCompletionBlock:validatorCompletionBlock];
}

#pragma mark - Camera

- (void)setPointOfView:(std::shared_ptr<VRONode>)node {
    _renderer->setPointOfView(node);
}

#pragma mark - Scene Loading

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController {
    [self setSceneController:sceneController duration:0 timingFunction:VROTimingFunctionType::EaseIn];
}

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController
                  duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController, seconds, timingFunctionType, _driver);
}

#pragma mark - Getters

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
}

- (std::shared_ptr<VRORenderer>)renderer {
    return _renderer;
}

#pragma mark - Rendering

- (void)drawRect:(CGRect)rect {
    @autoreleasepool {
        if (self.suspended) {
            [self renderSuspended];
        }
        else {
            [self renderFrame];
        }
    }
    
    ++_frame;
    ALLOCATION_TRACKER_PRINT();
}

- (void)renderFrame {
    glEnable(GL_DEPTH_TEST);    
    _driver->setCullMode(VROCullMode::Back);
    
    VROViewport viewport(0, 0, self.bounds.size.width  * self.contentScaleFactor,
                               self.bounds.size.height * self.contentScaleFactor);

    /*
     The viewport can be 0, if say in React Native, the user accidentally messes up their
     styles and React Native lays the view out with 0 width or height. No use rendering
     in this case.
     */
    if (viewport.getWidth() == 0 || viewport.getHeight() == 0) {
        return;
    }
    
    VROFieldOfView fov = _renderer->computeUserFieldOfView(viewport.getWidth(), viewport.getHeight());
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    
    _renderer->prepareFrame(_frame, viewport, fov, VROMatrix4f::identity(), projection, _driver);
    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    _renderer->renderEye(VROEyeType::Monocular, _renderer->getLookAtMatrix(), projection, viewport, _driver);
    _renderer->renderHUD(VROEyeType::Monocular, VROMatrix4f::identity(), projection, _driver);
    _renderer->endFrame(_driver);
}

- (void)renderSuspended {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    double newTime = VROTimeCurrentSeconds();
    // notify the user about bad keys 5 times a second (every 200ms/.2s)
    if (newTime - _suspendedNotificationTime > .2) {
        perr("Renderer suspended! Do you have a valid key?");
        _suspendedNotificationTime = newTime;
    }
}

- (void)recenterTracking {
    // No-op in Scene view
}

@end
