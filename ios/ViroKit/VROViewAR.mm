//
//  VROViewAR.m
//  ViroRenderer
//
//  Created by Raj Advani on 5/31/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROViewAR.h"
#import "VRORenderer.h"
#import "VROSceneController.h"
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
#import "VROARComponentManager.h"
#import "VROInputControllerARiOS.h"
#import "VROProjector.h"

@interface VROViewAR () {
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    std::shared_ptr<VROSurface> _cameraBackground;
    std::shared_ptr<VROARSession> _arSession;
    std::shared_ptr<VRONode> _pointOfView;
    std::shared_ptr<VROARComponentManager> _arComponentManager;
    std::shared_ptr<VROInputControllerARiOS> _inputController;
    
    CADisplayLink *_displayLink;
    int _frame;
    
    double _suspendedNotificationTime;
}

@property (readwrite, nonatomic) id <VROApiKeyValidator> keyValidator;

@end

@implementation VROViewAR

@dynamic renderDelegate;
@dynamic sceneController;

#pragma mark - Initialization

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame context:(EAGLContext *)context {
    self = [super initWithFrame:frame context:context];
    if (self) {
        [self initRenderer];
    }
    return self;
}

- (void)initRenderer {
    // TODO DisplayLink maintains a strong reference to its target, we have to
    //      create a weak proxy or something similar!
    
    if (!self.context) {
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        self.context = context;
    }
    
    /*
     Setup the animation loop for the GLKView.
     */
    _displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(display)];
    [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
    
    /*
     Setup the GLKView.
     */
    self.enableSetNeedsDisplay = NO;
    self.drawableColorFormat = GLKViewDrawableColorFormatRGB565;
    self.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    self.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    self.drawableMultisample = GLKViewDrawableMultisample4X;
    
    [EAGLContext setCurrentContext:self.context];
    
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
    _frame = 0;
    _gvrAudio = std::make_shared<gvr::AudioApi>();
    _gvrAudio->Init(GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    _driver = std::make_shared<VRODriverOpenGLiOS>(self.context, _gvrAudio);
    _suspendedNotificationTime = VROTimeCurrentSeconds();
    _inputController = std::make_shared<VROInputControllerARiOS>(self.bounds.size.width * self.contentScaleFactor,
                                                                 self.bounds.size.height * self.contentScaleFactor);
    _renderer = std::make_shared<VRORenderer>(_inputController);
    _inputController->setRenderer(_renderer);
    
    /*
     Create AR session.
     */
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
        _arSession = std::make_shared<VROARSessioniOS>(VROTrackingType::DOF6, _driver);
#else
        _arSession = std::make_shared<VROARSessionInertial>(VROTrackingType::DOF3, _driver);
#endif
    _arSession->setOrientation(VROConvert::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]));
    
    /*
     Create AR component manager and set it as the delegate to the AR session.
     */
    _arComponentManager = std::make_shared<VROARComponentManager>();
    _arSession->setDelegate(_arComponentManager);
    // TODO: remove the following line when we refactor VROARAnchor
    _arComponentManager->setSession(_arSession);
    
    /*
     Set the point of view to a special node that will follow the user's
     real position.
     */
    _pointOfView = std::make_shared<VRONode>();
    _pointOfView->setCamera(std::make_shared<VRONodeCamera>());
    _renderer->setPointOfView(_pointOfView);
    
    self.keyValidator = [[VROApiKeyValidatorDynamo alloc] init];
    
    UILongPressGestureRecognizer *longPress = [[UILongPressGestureRecognizer alloc] initWithTarget:self
                                                                                    action:@selector(handleLongPress:)];
    longPress.minimumPressDuration = 0;
    [self addGestureRecognizer:longPress];
}

- (void)handleLongPress:(UILongPressGestureRecognizer *)recognizer {
    CGPoint location = [recognizer locationInView:[recognizer.view superview]];
    
    VROVector3f viewportTouchPos = VROVector3f(location.x * self.contentScaleFactor, location.y * self.contentScaleFactor);
    
    if (recognizer.state == UIGestureRecognizerStateBegan) {
        _inputController->onScreenTouchDown(viewportTouchPos);
    } else if (recognizer.state == UIGestureRecognizerStateEnded) {
        _inputController->onScreenTouchUp(viewportTouchPos);
    } else {
        _inputController->onScreenTouchMove(viewportTouchPos);
        return;
    }
    
    if (_arSession && _arSession->isReady()) {
        std::unique_ptr<VROARFrame> &frame = _arSession->getLastFrame();
        std::vector<VROARHitTestResult> results = frame->hitTest(location.x * self.contentScaleFactor,
                                                                 location.y * self.contentScaleFactor,
                       { VROARHitTestResultType::ExistingPlaneUsingExtent,
                         VROARHitTestResultType::ExistingPlane,
                         VROARHitTestResultType::EstimatedHorizontalPlane,
                         VROARHitTestResultType::FeaturePoint });
        
        for (VROARHitTestResult &result : results) {
            if (self.tapHandler) {
                self.tapHandler(result, _arSession, _sceneController->getScene());
            }
        }
    }
}
// End TODO

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Settings and Notifications

- (void)orientationDidChange:(NSNotification *)notification {
    if (_cameraBackground) {
        _cameraBackground->setX(self.bounds.size.width * self.contentScaleFactor / 2.0);
        _cameraBackground->setY(self.bounds.size.height * self.contentScaleFactor / 2.0);
        _cameraBackground->setWidth(self.bounds.size.width * self.contentScaleFactor);
        _cameraBackground->setHeight(self.bounds.size.height * self.contentScaleFactor);
    }
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
    [self.keyValidator validateApiKey:apiKey withCompletionBlock:validatorCompletionBlock];
}

#pragma mark - Camera

- (void)setPointOfView:(std::shared_ptr<VRONode>)node {
    pabort("May not set POV in AR mode");
}

- (void)layoutSubviews {
    [super layoutSubviews];
    _renderer->updateRenderViewSize(self.bounds.size.width, self.bounds.size.height);
}

#pragma mark - Scene Loading

- (void)setSceneController:(std::shared_ptr<VROSceneController>) sceneController {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController, _driver);
    
    // Reset the camera background for the new scene
    _cameraBackground.reset();
}

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController
                  duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController, seconds, timingFunctionType, _driver);
    
    // Reset the camera background for the new scene
    _cameraBackground.reset();
}

#pragma mark - Frame Listeners

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
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
    /*
     Setup GL state.
     */
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE); // Must enable writes to clear depth buffer
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    VROViewport viewport(0, 0, self.bounds.size.width  * self.contentScaleFactor,
                               self.bounds.size.height * self.contentScaleFactor);
    
    if (_sceneController) {
        if (!_cameraBackground) {
            [self initARSessionWithViewport:viewport scene:_sceneController->getScene()];
        }
    }
    
    if (_arSession->isReady()) {
        // TODO Only on viewport change
        _arSession->setViewport(viewport);
        
        // TODO Only on first run of isReady()
        _sceneController->getScene()->setBackground(_cameraBackground);

        /*
         Retrieve transforms from the AR session.
         */
        std::unique_ptr<VROARFrame> &frame = _arSession->updateFrame();
        const std::shared_ptr<VROARCamera> camera = frame->getCamera();
        
        VROFieldOfView fov;
        VROMatrix4f projection = camera->getProjection(viewport, kZNear, _renderer->getFarClippingPlane(), &fov);
        VROMatrix4f rotation = camera->getRotation();
        VROVector3f position = camera->getPosition();
        
        // TODO Only on orientation change
        VROMatrix4f backgroundTransform = frame->getViewportToCameraImageTransform();
        _cameraBackground->setTexcoordTransform(backgroundTransform);
        
        /*
         Render the 3D scene.
         */
        _pointOfView->getCamera()->setPosition(position);
        _renderer->prepareFrame(_frame, viewport, fov, rotation, projection, _driver);
        
        glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
        _renderer->renderEye(VROEyeType::Monocular, VROMatrix4f::identity(), projection, _driver);
        _renderer->endFrame(_driver);
    }
    else {
        /*
         Render black while waiting for the AR session to initialize.
         */
        VROFieldOfView fov = _renderer->computeMonoFOV(viewport.getWidth(), viewport.getHeight());
        VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
        
        _renderer->prepareFrame(_frame, viewport, fov, VROMatrix4f::identity(), projection, _driver);
        glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
        _renderer->renderEye(VROEyeType::Monocular, VROMatrix4f::identity(), projection, _driver);
        _renderer->endFrame(_driver);
    }
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
    material->setReadsFromDepthBuffer(false);
    
    _arSession->setScene(scene);
    _arSession->setViewport(viewport);
    _arSession->setAnchorDetection({ VROAnchorDetection::PlanesHorizontal });
    _arSession->run();
    
    // TODO: change the scene to VROARScene in this this class?
    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(scene);
    arScene->setARComponentManager(_arComponentManager);
}

- (void)recenterTracking {
    // TODO Implement this, try to share code with VROSceneRendererCardboardOpenGL; maybe
    //      move the functionality into VRORenderer
}

@end
