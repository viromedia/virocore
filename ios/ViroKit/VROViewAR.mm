//
//  VROViewAR.m
//  ViroRenderer
//
//  Created by Raj Advani on 5/31/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROViewAR.h"
#import "VRORenderer.h"
#import "VROARSceneController.h"
#import "VRORenderDelegateiOS.h"
#import "VROTime.h"
#import "VROEye.h"
#import "VRODriverOpenGLiOS.h"
#import "VROApiKeyValidator.h"
#import "VROApiKeyValidatorDynamo.h"
#import "VROARSessioniOS.h" 
#import "VROARSessionInertial.h"
#import "VROARCamera.h"
#import "VROARAnchor.h"
#import "VROARFrame.h"
#import "VROConvert.h"
#import "VROARHitTestResult.h"
#import "VRONodeCamera.h"
#import "vr/gvr/capi/include/gvr_audio.h"
#import "VROARScene.h"
#import "VROChoreographer.h"
#import "VROInputControllerAR.h"
#import "VROProjector.h"
#import "VROWeakProxy.h"
#import "VROViewRecorder.h"
#import "VROMetricsRecorder.h"
#import "VROARCameraInertial.h"
#import "VRODeviceUtil.h"
#import "VROCameraTexture.h"

static VROVector3f const kZeroVector = VROVector3f();

@interface VROViewAR () {
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROARSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<VROSurface> _cameraBackground;
    std::shared_ptr<VROARSession> _arSession;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROInputControllerAR> _inputController;
    
    CADisplayLink *_displayLink;
    int _frame;
    double _suspendedNotificationTime;

    VROWorldAlignment _worldAlignment;
}

@property (readwrite, nonatomic) id <VROApiKeyValidator> keyValidator;
@property (readwrite, nonatomic) VROViewRecorder *viewRecorder;
@property (readwrite, nonatomic) VROTrackingType trackingType;

// Image Tracking Output
@property (readwrite, nonatomic) UITextView *trackerStatusText;
@property (readwrite, nonatomic) UIImageView *trackerOutputView;
@property (readwrite, nonatomic) UITextView *trackerOutputText;
@property (readwrite, nonatomic) CGFloat trackerViewScale;
@property (readwrite, nonatomic) CGFloat textHeight;

// Debug view for drawing with CoreGraphics
@property (readwrite, nonatomic) UIView *glassView;

@end

@implementation VROViewAR

@dynamic renderDelegate;
@dynamic sceneController;

#pragma mark - Initialization

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        _suspended = YES;
        _worldAlignment = VROWorldAlignment::Gravity;
        
        VRORendererConfiguration config;
        [self initRenderer:config];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame
                       config:(VRORendererConfiguration)config
                      context:(EAGLContext *)context
               worldAlignment:(VROWorldAlignment)worldAlignment {
    return [self initWithFrame:frame config:config context:context worldAlignment:worldAlignment trackingType:VROTrackingType::DOF6];
}

- (instancetype)initWithFrame:(CGRect)frame
                       config:(VRORendererConfiguration)config
                      context:(EAGLContext *)context
               worldAlignment:(VROWorldAlignment)worldAlignment
                 trackingType:(VROTrackingType)trackingType {
    self = [super initWithFrame:frame context:context];
    if (self) {
        _suspended = YES;
        _worldAlignment = worldAlignment;
        _trackingType = trackingType;
        if (_trackingType == VROTrackingType::Front) {
            _cameraPosition = VROCameraPosition::Front;
        } else {
            _cameraPosition = VROCameraPosition::Back;
        }
        [self initRenderer:config];
    }
    return self;
}

- (void)setFrame:(CGRect)frame {
    [super setFrame:frame];
    
    // If the frame changes, then update the _camera background to match
    if (_cameraBackground) {
        _cameraBackground->setX(self.frame.size.width * self.contentScaleFactor / 2.0);
        _cameraBackground->setY(self.frame.size.height * self.contentScaleFactor / 2.0);
        _cameraBackground->setWidth(self.frame.size.width * self.contentScaleFactor);
        _cameraBackground->setHeight(self.frame.size.height * self.contentScaleFactor);
    }

    if (_inputController) {
        _inputController->setViewportSize(self.frame.size.width * self.contentScaleFactor,
                                          self.frame.size.height * self.contentScaleFactor);
    }

    if (_trackerOutputView) {
        float width = self.frame.size.width * _trackerViewScale;
        float height = self.frame.size.height * _trackerViewScale;
        _trackerOutputView.frame = CGRectMake(self.frame.size.width - width, self.frame.size.height - height, width, height);
    }
    
    if (_trackerOutputText) {
        _trackerOutputText.frame = CGRectMake(0, self.frame.size.height - _textHeight, self.frame.size.width, _textHeight);

    }
}

- (void)initRenderer:(VRORendererConfiguration)config {
    VROPlatformSetType(VROPlatformType::iOSARKit);
    if (!self.context) {
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        self.context = context;
    }
    VROPlatformSetEAGLContext(self.context);
    VROThreadRestricted::setThread(VROThreadName::Renderer);
    
    /*
     Setup the GLKView.
     */
    self.enableSetNeedsDisplay = NO;
    self.drawableColorFormat = GLKViewDrawableColorFormatSRGBA8888;
    self.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    self.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    if (config.enableMultisampling) {
        self.drawableMultisample = GLKViewDrawableMultisample4X;
    }
    
    [EAGLContext setCurrentContext:self.context];

    /*
     Setup the animation loop for the GLKView.
     */
    VROWeakProxy *proxy = [VROWeakProxy weakProxyForObject:self];
    _displayLink = [CADisplayLink displayLinkWithTarget:proxy selector:@selector(display)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];

#if VRO_POSEMOJI
    VRODeviceUtil *device = [[VRODeviceUtil alloc] init];
    if (![device isBionicA12]) {
        _displayLink.preferredFramesPerSecond = 30;
        pinfo("Preferred FPS 30");
    } else {
        pinfo("Preferred FPS 60");
    }
#endif

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
                                                              self.frame.size.height * self.contentScaleFactor,
                                                              _driver);

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
    
    /*
     Create AR session checking if an ARKit class and one of our classes have been defined. If not, then load VROARSessionInertial,
     otherwise create a VROARSessioniOS w/ 6DOF tracking.
     */
    if (NSClassFromString(@"ARSession") == nil) {
        _arSession = std::make_shared<VROARSessionInertial>(VROTrackingType::DOF3, _driver);
    } else if (_trackingType == VROTrackingType::Front) {
        _arSession = std::make_shared<VROARSessionInertial>(VROTrackingType::Front, _driver);
    } else {
        _arSession = std::make_shared<VROARSessioniOS>(_trackingType, _worldAlignment, _driver);
    }

    _arSession->setOrientation(VROConvert::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]));
    _inputController->setSession(std::dynamic_pointer_cast<VROARSession>(_arSession));

#if ENABLE_OPENCV
    // Below code is used to help us debug OpenCV tracking
    if (NSClassFromString(@"ARSession") != nil) {
        _trackerViewScale = .50; // scale w.r.t. the entire screen of the output image w/ the edges drawn (from the bottom right)
        _textHeight = 75; // height of the output text view at the bottom (blue box w/ text)

        // Create a new ImageView for displaying image tracking
        float width = self.frame.size.width * _trackerViewScale;
        float height = self.frame.size.height * _trackerViewScale;
        _trackerOutputView = [[UIImageView alloc] initWithFrame:CGRectMake(self.frame.size.width - width, self.frame.size.height - height, width, height)];
        _trackerOutputView.contentMode = UIViewContentModeScaleToFill;
        _trackerOutputView.alpha = .65;
        
        std::shared_ptr<VROARSessioniOS> arSessioniOS = std::dynamic_pointer_cast<VROARSessioniOS>(_arSession);
        
        // Create a text view for printing out image tracking stuff
        _trackerOutputText = [[UITextView alloc] initWithFrame:CGRectMake(0, self.frame.size.height - _textHeight, self.frame.size.width, _textHeight)];
        _trackerOutputText.backgroundColor = UIColor.blueColor;
        _trackerOutputText.textColor = UIColor.whiteColor;
        _trackerOutputText.alpha = .6;
        _trackerOutputText.textAlignment = NSTextAlignmentCenter;
        _trackerOutputText.font = [UIFont systemFontOfSize:12];
        _trackerOutputText.editable = NO;
        
        UITapGestureRecognizer *singleFingerTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleOutputTextTap:)];
        [_trackerOutputText addGestureRecognizer:singleFingerTap];

        // Add the views to the view.
        [self addSubview:_trackerOutputText];
        [self addSubview:_trackerOutputView];
        
        arSessioniOS->setTrackerOutputView(_trackerOutputView);
        arSessioniOS->setTrackerOutputText(_trackerOutputText);
    }
#endif
    
    /*
     Set the point of view to a special node that will follow the user's
     real position.
     */
    _pointOfView = std::make_shared<VRONode>();
    _pointOfView->setCamera(std::make_shared<VRONodeCamera>());
    _renderer->setPointOfView(_pointOfView);
    
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

#if ENABLE_OPENCV
- (void)handleOutputTextTap:(UITapGestureRecognizer *)recognizer {
    std::shared_ptr<VROARSessioniOS> arSessioniOS = std::dynamic_pointer_cast<VROARSessioniOS>(_arSession);
    arSessioniOS->outputTextTapped();
}
#endif /* ENABLE_OPNECV */

#pragma mark Gesture handlers

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
    if (_arSession) {
        _arSession.reset();
    }
    if (_sceneController) {
        _sceneController->getScene()->getRootNode()->deleteGL();
    }
}

- (void)setPaused:(BOOL)paused {
    [_displayLink setPaused:paused];
}

- (BOOL)setShadowsEnabled:(BOOL)enabled {
    return _renderer->setShadowsEnabled(enabled);
}

- (BOOL)setHDREnabled:(BOOL)enabled {
    return _renderer->setHDREnabled(enabled);
}

- (BOOL)setPBREnabled:(BOOL)enabled {
    return _renderer->setPBREnabled(enabled);
}

- (BOOL)setBloomEnabled:(BOOL)enabled {
    return _renderer->setBloomEnabled(enabled);
}

- (VROVector3f)unprojectPoint:(VROVector3f)point {
    return _renderer->unprojectPoint(point);
}

- (VROVector3f)projectPoint:(VROVector3f)point {
    return _renderer->projectPoint(point);
}

#pragma mark - Recording and Screenshots

- (void)startVideoRecording:(NSString *)fileName
           saveToCameraRoll:(BOOL)saveToCamera
                 errorBlock:(VROViewRecordingErrorBlock)errorBlock {
    [self.viewRecorder startVideoRecording:fileName saveToCameraRoll:saveToCamera errorBlock:errorBlock];
}

- (void)startVideoRecording:(NSString *)fileName
              withWatermark:(UIImage *)watermarkImage
                  withFrame:(CGRect)watermarkFrame
           saveToCameraRoll:(BOOL)saveToCamera
                 errorBlock:(VROViewRecordingErrorBlock)errorBlock {
    [self.viewRecorder startVideoRecording:fileName withWatermark:watermarkImage withFrame:watermarkFrame saveToCameraRoll:saveToCamera errorBlock:errorBlock];
}

- (void)stopVideoRecordingWithHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    [self.viewRecorder stopVideoRecordingWithHandler:completionHandler];
}

- (void)takeScreenshot:(NSString *)fileName
      saveToCameraRoll:(BOOL)saveToCamera
 withCompletionHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    [self.viewRecorder takeScreenshot:fileName saveToCameraRoll:saveToCamera withCompletionHandler:completionHandler];
}

#pragma mark - AR Functions

- (std::vector<std::shared_ptr<VROARHitTestResult>>)performARHitTest:(VROVector3f)ray {
    // check that the ray is in front of the camera
    VROVector3f cameraForward = _renderer->getCamera().getForward();
    if (cameraForward.dot(ray) <= 0) {
        return {};
    }
    
    VROVector3f worldPoint = _renderer->getCamera().getPosition() + ray.normalize();
    VROVector3f screenPoint = _renderer->projectPoint(worldPoint);

    return [self performARHitTestWithPoint:screenPoint.x y:screenPoint.y];
}

- (std::vector<std::shared_ptr<VROARHitTestResult>>)performARHitTestWithPoint:(int)x y:(int)y {
    int viewportArr[4] = {0, 0,
        (int) (self.bounds.size.width  * self.contentScaleFactor),
        (int) (self.bounds.size.height * self.contentScaleFactor)};

    // check the 2D point, perform and return the results from the AR hit test
    std::unique_ptr<VROARFrame> &frame = _arSession->getLastFrame();
    if (frame && x >= 0 && x <= viewportArr[2] && y >= 0 && y <= viewportArr[3]) {
        std::vector<std::shared_ptr<VROARHitTestResult>> results = frame->hitTest(x, y,
                                                                 { VROARHitTestResultType::ExistingPlaneUsingExtent,
                                                                     VROARHitTestResultType::ExistingPlane,
                                                                     VROARHitTestResultType::EstimatedHorizontalPlane,
                                                                     VROARHitTestResultType::FeaturePoint });
        return results;
    }

    return {};
}

- (std::shared_ptr<VROARSession>)getARSession {
    return _arSession;
}

#pragma mark - Settings and Notifications

- (void)orientationDidChange:(NSNotification *)notification {
    // the _cameraBackground will be updated if/when the frame is actually set.
    _arSession->setOrientation(VROConvert::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]));
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    _displayLink.paused = YES;
    _arSession->pause();
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    _displayLink.paused = NO;
    _arSession->run(); 
}

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    _renderDelegateWrapper = std::make_shared<VRORenderDelegateiOS>(renderDelegate);
    _renderer->setDelegate(_renderDelegateWrapper);
}

- (void)setARSessionDelegate:(std::shared_ptr<VROARSessionDelegate>)delegate {
    _arSession->setDelegate(delegate);
}

- (void)setVrMode:(BOOL)enabled {
    // No-op in AR mode
}

- (void)setDebugHUDEnabled:(BOOL)enabled {
    _renderer->setDebugHUDEnabled(enabled);
}

- (NSString *)getPlatform {
    return @"ar";
}

- (NSString *)getHeadset {
    return [NSString stringWithUTF8String:_inputController->getHeadset().c_str()];
}

- (NSString *)getController {
    return [NSString stringWithUTF8String:_inputController->getController().c_str()];
}

- (void)setDebugDrawDelegate:(NSObject<VRODebugDrawDelegate> *)debugDrawDelegate {
    self.glassView = [[VROGlassView alloc] initWithFrame:self.bounds delegate:debugDrawDelegate];
    [self addSubview:self.glassView];
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
    // Record keen_io metrics
    [[VROMetricsRecorder sharedClientWithViewType:@"AR" platform:@"arkit"] recordEvent:@"renderer_init"];
}

#pragma mark - Camera

- (void)setPointOfView:(std::shared_ptr<VRONode>)node {
    self.renderer->setPointOfView(node);
}

#pragma mark - Scene Loading

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController {
    [self setSceneController:sceneController duration:0 timingFunction:VROTimingFunctionType::EaseIn];
}

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController
                  duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    _sceneController = std::dynamic_pointer_cast<VROARSceneController>(sceneController);
    passert_msg (_sceneController != nullptr, "AR View requires an AR Scene Controller!");

    _renderer->setSceneController(sceneController, seconds, timingFunctionType, _driver);

    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    passert_msg (arScene != nullptr, "AR View requires an AR Scene!");
    
    _arSession->setScene(arScene);
    _arSession->setDelegate(arScene->getSessionDelegate());
    arScene->setARSession(_arSession);
    arScene->addNode(_pointOfView);
    arScene->setDriver(_driver);

    // Reset the camera background for the new scene
    _cameraBackground.reset();
}

#pragma mark - Getters

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
}

- (std::shared_ptr<VRORenderer>)renderer {
    return _renderer;
}

- (std::shared_ptr<VROChoreographer>)choreographer {
    return _renderer->getChoreographer();
}

#pragma mark - Rendering

- (void)drawRect:(CGRect)rect {
    // Clear error state
    GL ();
    
    @autoreleasepool {
        if (self.suspended) {
            [self renderSuspended];
        } else {
            [self renderFrame];
        }
    }
    if (_glassView) {
        [_glassView setNeedsDisplay];
    }
    
    ++_frame;
    ALLOCATION_TRACKER_PRINT();
}

- (void)renderFrame {
    if (!_arSession) {
        return;
    }
    
    /*
     Setup GL state.
     */
    glEnable(GL_DEPTH_TEST);    
    _driver->setCullMode(VROCullMode::Back);
    
    VROViewport viewport(0, 0, self.bounds.size.width  * self.contentScaleFactor,
                               self.bounds.size.height * self.contentScaleFactor);
    
    /*
     Attempt to initialize the ARSession if we have not yet done so.
     */
    if (_sceneController) {
        if (!_cameraBackground) {
            [self initARSessionWithViewport:viewport scene:_sceneController->getScene()];
        }
    }

    /*
     The viewport can be 0, if say in React Native, the user accidentally messes up their
     styles and React Native lays the view out with 0 width or height. No use rendering
     in this case.
     */
    if (viewport.getWidth() == 0 || viewport.getHeight() == 0) {
        return;
    }

    /*
     If the ARSession is not yet ready, render black.
     */
    if (!_arSession->isReady()) {
        [self renderWithoutTracking:viewport];
        return;
    }
    
    /*
     ARSession is ready (meaning at least one frame has been produced).
     */
    _arSession->setViewport(viewport);
    
    /*
     When using the front-facing camera with preview layer, we don't use our own background;
     instead the camera is rendered via an AVCaptureVideoPreviewLayer.
     */
    if (!_sceneController->getScene()->getRootNode()->getBackground() &&
        (_trackingType != VROTrackingType::Front || !kInertialRenderCameraUsingPreviewLayer)) {
        _sceneController->getScene()->getRootNode()->setBackground(_cameraBackground);
    }

    /*
     Retrieve transforms from the AR session.
     */
    const std::unique_ptr<VROARFrame> &frame = _arSession->updateFrame();
    const std::shared_ptr<VROARCamera> camera = frame->getCamera();
    
    /*
     Update the AR camera background transform (maps the camera background to our scene).
     If we're in front-facing mode, we also have to mirror the camera background.
     */
    VROMatrix4f backgroundTransform = frame->getViewportToCameraImageTransform();
    if (_trackingType == VROTrackingType::Front) {
        VROMatrix4f mirrorX;
        mirrorX.scale(-1, 1, 1);
        backgroundTransform = mirrorX * backgroundTransform;
    }
    _cameraBackground->setTexcoordTransform(backgroundTransform);

    /*
     Notify the current ARScene with the ARCamera's tracking state.
     */
    if (_sceneController) {
        std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
        passert_msg (arScene != nullptr, "AR View requires an AR Scene!");
        arScene->setTrackingState(camera->getTrackingState(), camera->getLimitedTrackingStateReason(), false);
    }

    /*
     Render the scene.
     */
    [self renderWithTracking:camera withFrame:frame withViewport:viewport];

    /*
     Notify scene of the updated ambient light estimates.
     */
    std::shared_ptr<VROARScene> scene = std::dynamic_pointer_cast<VROARScene>(_arSession->getScene());
    scene->updateAmbientLight(frame->getAmbientLightIntensity(), frame->getAmbientLightColor());
}

- (void)renderWithTracking:(const std::shared_ptr<VROARCamera>) camera
                 withFrame:(const std::unique_ptr<VROARFrame> &) frame
              withViewport:(VROViewport)viewport {
    VROFieldOfView fov;
    VROMatrix4f projection = camera->getProjection(viewport, kZNear, _renderer->getFarClippingPlane(), &fov);
    VROMatrix4f rotation = camera->getRotation();
    VROVector3f position = camera->getPosition();
    
    _pointOfView->getCamera()->setPosition(position);
    _renderer->prepareFrame(_frame, viewport, fov, rotation, projection, _driver);
    _renderer->renderEye(VROEyeType::Monocular, _renderer->getLookAtMatrix(), projection, viewport, _driver);
    _renderer->renderHUD(VROEyeType::Monocular, VROMatrix4f::identity(), projection, _driver);
    _renderer->endFrame(_driver);
}

/*
 Render black while waiting for the AR session to initialize.
 */
- (void)renderWithoutTracking:(VROViewport)viewport {
    VROFieldOfView fov = _renderer->computeUserFieldOfView(viewport.getWidth(), viewport.getHeight());
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    
    _renderer->prepareFrame(_frame, viewport, fov, VROMatrix4f::identity(), projection, _driver);
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

- (void)initARSessionWithViewport:(VROViewport)viewport scene:(std::shared_ptr<VROScene>)scene {
    _cameraBackground = VROSurface::createSurface(viewport.getX() + viewport.getWidth()  / 2.0,
                                                  viewport.getY() + viewport.getHeight() / 2.0,
                                                  viewport.getWidth(), viewport.getHeight(),
                                                  0, 0, 1, 1);
    _cameraBackground->setScreenSpace(true);
    _cameraBackground->setName("Camera");
    
    std::shared_ptr<VROMaterial> material = _cameraBackground->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Constant);
    material->getDiffuse().setTexture(_arSession->getCameraBackgroundTexture());
    material->setWritesToDepthBuffer(false);
    material->setNeedsToneMapping(false);
    material->setCullMode(VROCullMode::None); // Required for mirrored mode when using front-facing camera
    
    _arSession->setViewport(viewport);
    _arSession->run();
}

+ (BOOL) isARSupported {
  if (@available(iOS 11, *)) {
    return [ARConfiguration isSupported];
  }
  return false;
}

- (void)recenterTracking {
    // TODO Implement this, try to share code with VROSceneRendererCardboardOpenGL; maybe
    //      move the functionality into VRORenderer
}

@end

@implementation VROGlassView

- (id)initWithFrame:(CGRect)frame delegate:(NSObject<VRODebugDrawDelegate> *)delegate {
    self = [super initWithFrame:frame];
    if (self) {
        _debugDrawDelegate = delegate;
        self.backgroundColor = [UIColor clearColor];
    }
    return self;
}

- (void)drawRect:(CGRect)rect {
    if (self.debugDrawDelegate) {
        [self.debugDrawDelegate drawRect];
    }
}

@end
