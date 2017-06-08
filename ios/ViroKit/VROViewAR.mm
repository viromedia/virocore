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
#import "VROInputControllerCardboardiOS.h"
#import "VROARSessioniOS.h"
#import "VROARSessionInertial.h"
#import "VROARCamera.h"
#import "VROARAnchor.h"
#import "VROARFrame.h"
#import "VROCameraTextureiOS.h"
#import "vr/gvr/capi/include/gvr_audio.h"

@interface VROViewAR () {
    std::shared_ptr<VRORenderer> _renderer;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    std::shared_ptr<VROSurface> _cameraBackground;
    std::shared_ptr<VROARSession> _arSession;
    
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
    _renderer = std::make_shared<VRORenderer>(std::make_shared<VROInputControllerCardboardiOS>());
    
    /*
     Create AR session.
     */
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 110000
        _arSession = std::make_shared<VROARSessioniOS>(VROTrackingType::DOF6, _driver);
#else
        _arSession = std::make_shared<VROARSessionInertial>(VROTrackingType::DOF3, _driver);
#endif
    _arSession->setOrientation(VROCameraTextureiOS::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]));
    _arSession->run();
    
    self.keyValidator = [[VROApiKeyValidatorDynamo alloc] init];
}

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
    _arSession->setOrientation(VROCameraTextureiOS::toCameraOrientation([[UIApplication sharedApplication] statusBarOrientation]));
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
    _renderer->setPointOfView(node);
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
    VROViewport viewport(0, 0, self.bounds.size.width  * self.contentScaleFactor,
                               self.bounds.size.height * self.contentScaleFactor);
    
    VROFieldOfView fov = VRORenderer::computeMonoFOV(viewport.getWidth(), viewport.getHeight());
    VROMatrix4f projection = fov.toPerspectiveProjection(kZNear, _renderer->getFarClippingPlane());
    VROMatrix4f rotation;
    
    /*
     Retrieve transforms from the AR session.
     */
    if (_sceneController) {
        if (!_cameraBackground) {
            [self initCameraBackgroundWithViewport:viewport forScene:_sceneController->getScene()];
        }
    
        if (_arSession->isReady()) {
            _arSession->setViewport(viewport);
            
            std::unique_ptr<VROARFrame> &frame = _arSession->updateFrame();
            
            const std::shared_ptr<VROARCamera> camera = frame->getCamera();
            rotation = camera->getRotation();
            projection = camera->getProjection(viewport, kZNear, _renderer->getFarClippingPlane(), &fov);
            
            VROMatrix4f backgroundTransform = frame->getBackgroundTexcoordTransform();
            _cameraBackground->setTexcoordTransform(backgroundTransform);
        }
    }
    
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
    
    /*
     Render the 3D scene.
     */
    _renderer->prepareFrame(_frame, viewport, fov, rotation, projection, _driver);
    
    VROMatrix4f eyeFromHeadMatrix; // Identity
    
    glViewport(viewport.getX(), viewport.getY(), viewport.getWidth(), viewport.getHeight());
    _renderer->renderEye(VROEyeType::Monocular, eyeFromHeadMatrix, projection, _driver);
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

- (void)initCameraBackgroundWithViewport:(VROViewport)viewport forScene:(std::shared_ptr<VROScene>)scene {
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

    scene->setBackground(_cameraBackground);
}

- (void)recenterTracking {
    // TODO Implement this, try to share code with VROSceneRendererCardboardOpenGL; maybe
    //      move the functionality into VRORenderer
}

@end
