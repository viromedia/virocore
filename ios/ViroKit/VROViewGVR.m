//
//  VROViewGVR.m
//  ViroRenderer
//
//  Created by Raj Advani on 4/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROViewGVR.h"
#import <memory>
#import "VRORenderer.h"
#import "VROWeakProxy.h"
#import "VROPlatformUtil.h"
#import "VRORenderer.h"
#import "VROFieldOfView.h"
#import "VROViewport.h"
#import "VROReticle.h"
#import "VROApiKeyValidatorDynamo.h"
#import "VROAllocationTracker.h"
#import "VRORenderDelegateiOS.h"
#import "VROInputControllerCardboardiOS.h"
#import "VRODriverOpenGLiOSGVR.h"
#import "VROSceneRendererGVR.h"
#import "VROChoreographer.h"
#import "GVROverlayView.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "VROMetricsRecorder.h"

@interface VROOverlayViewDelegate : NSObject <GVROverlayViewDelegate>

@property (readwrite, nonatomic, weak) VROViewGVR *view;

@end

@interface VROViewGVR () {
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    CADisplayLink *_displayLink;
    GVROverlayView *_overlayView;
}

@property (readwrite, nonatomic) std::shared_ptr<VRORenderer> renderer;
@property (readwrite, nonatomic) std::shared_ptr<VROSceneRendererGVR> sceneRenderer;
@property (readwrite, nonatomic) id <VROApiKeyValidator> keyValidator;
@property (readwrite, nonatomic) BOOL initialized;
@property (readwrite, nonatomic) BOOL VRModeEnabled;
@property (readwrite, nonatomic) VROOverlayViewDelegate *gvrDelegate;

@end

@implementation VROViewGVR

@dynamic renderDelegate;
@dynamic sceneController;

#pragma mark - Initialization

- (instancetype)initWithCoder:(NSCoder *)coder {
    self = [super initWithCoder:coder];
    if (self) {
        VRORendererConfiguration config;
        [self initRenderer:config];
    }
    return self;
}

- (instancetype)initWithFrame:(CGRect)frame config:(VRORendererConfiguration)config {
    self = [super initWithFrame:frame];
    if (self) {
        [self initRenderer:config];
    }
    return self;
}

- (void)dealloc {
    VROThreadRestricted::unsetThread();
}

- (void)setTestingMode:(BOOL)testingMode {
    _testingMode = testingMode;
    if (_testingMode) {
        [self setVrMode:true];
        _sceneRenderer->setSuspended(false);
    }
}

- (void)initRenderer:(VRORendererConfiguration)config {
    VROPlatformSetType(VROPlatformType::iOSCardboard);
    if (!self.context) {
        EAGLContext *context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        self.context = context;
    }
    VROThreadRestricted::setThread(VROThreadName::Renderer);
    
    _VRModeEnabled = YES;
    _paused = YES;
    
    _overlayView = [[GVROverlayView alloc] initWithFrame:self.bounds];
    _overlayView.hidesBackButton = NO;
    _overlayView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    [self addSubview:_overlayView];
    
    /*
     Create the delegate.
     */
    _gvrDelegate = [[VROOverlayViewDelegate alloc] init];
    _gvrDelegate.view = self;
    [_overlayView setDelegate:_gvrDelegate];
    
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
     Disable going to sleep, and setup notifications.
     */
    [UIApplication sharedApplication].idleTimerDisabled = YES;
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
    _driver = std::make_shared<VRODriverOpenGLiOSGVR>(self, self.context);
    _renderer = std::make_shared<VRORenderer>(config, std::make_shared<VROInputControllerCardboardiOS>());
    _sceneRenderer = std::make_shared<VROSceneRendererGVR>(self.bounds.size.width * self.contentScaleFactor,
                                                           self.bounds.size.height * self.contentScaleFactor,
                                                           [[UIApplication sharedApplication] statusBarOrientation],
                                                           self.contentScaleFactor, _renderer, _driver);
    self.keyValidator = [[VROApiKeyValidatorDynamo alloc] init];
    
    /*
     Add a tap gesture to handle viewer trigger action.
     */
    UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapGLView:)];
    [self addGestureRecognizer:tapGesture];
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    self.paused = YES;
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    self.paused = NO;
}

- (void)setVrMode:(BOOL)enabled {
    if (_VRModeEnabled != enabled) {
        _sceneRenderer->setVRModeEnabled(enabled);
        _VRModeEnabled = enabled;
    }
    [self updateOverlayView];
}

- (void)setPaused:(BOOL)paused {
    _paused = paused;
    if (_paused) {
        _renderer->getInputController()->onPause();
        _driver->pause();
        _sceneRenderer->pause();
    }
    else {
        _renderer->getInputController()->onResume();
        _driver->resume();
        _sceneRenderer->resume();
    }
    _displayLink.paused = (self.superview == nil || _paused);
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

- (void)didMoveToSuperview {
    [super didMoveToSuperview];
    if (self.superview) {
        [self startRenderer];
    }
    else {
        [self stopRenderer];
    }
}

- (void)layoutSubviews {
    [super layoutSubviews];
    
    // This sanity check fixes an issue where when we enter an app in portrait
    // mode then enter GVR, the height goes to zero and results in an invalid
    // framebuffer. This occurs because in VROViewControllerGVR we force the
    // orientation to landscape right, during which some race condition occurs
    // creating the distortion. (Note that we do this in VROViewControllerGVR
    // because of a separate GVR issue that results in a half-size viewport).
    // Effectively then this patches a bug caused by a patch of an underlying GVR
    // issue.
    if (self.bounds.size.height == 0 || self.bounds.size.width == 0) {
        NSLog(@"Invalid bounds on GVR view (%f, %f): making fullscreen",
              self.bounds.size.width, self.bounds.size.height);
        if (self.superview) {
            [self.superview setFrame:[UIScreen mainScreen].bounds];
            [self setFrame:self.superview.bounds];
            
            NSLog(@"Corrected superview bounds to full screen (%f, %f)",
                  self.superview.frame.size.width, self.superview.frame.size.height);
        }
    }
    
    if (!_initialized) {
        pinfo("Initializing GVR renderer with bounds (%f, %f)", self.bounds.size.width, self.bounds.size.height);
        _initialized = YES;
        [self bindDrawable];
        _sceneRenderer->initGL();
    }
    
    // This check is needed for some rare cases where we're getting layoutSubviews
    // invoked mid-orientation change, resulting in 0 width or height
    UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    _sceneRenderer->setSurfaceSize(self.bounds.size.width * self.contentScaleFactor,
                                   self.bounds.size.height * self.contentScaleFactor,
                                   orientation);
}

- (void)updateOverlayView {
    // Transition view is always shown when VR mode is toggled ON.
    _overlayView.hidesTransitionView = _overlayView.hidesTransitionView || !_VRModeEnabled;
    _overlayView.hidesSettingsButton = !_VRModeEnabled;
    _overlayView.hidesAlignmentMarker = !_VRModeEnabled;
    _overlayView.hidesFullscreenButton = !_VRModeEnabled;
    _overlayView.hidesCardboardButton = YES;
    
    [_overlayView setNeedsLayout];
}

- (void)didTapGLView:(id)sender {
    std::shared_ptr<VROInputControllerBase> baseController = _renderer->getInputController();
    std::shared_ptr<VROInputControllerCardboardiOS> cardboardController = std::dynamic_pointer_cast<VROInputControllerCardboardiOS>(baseController);
    cardboardController->onScreenClicked();
}

#pragma mark - Rendering

- (void)drawRect:(CGRect)rect {
    [super drawRect:rect];
    if (!_initialized) {
        pinfo("GVR not initialized, not drawing with bounds (%f, %f)",
              self.bounds.size.width, self.bounds.size.height);
        return;
    }
    @autoreleasepool {
        _sceneRenderer->onDrawFrame();
    }
}

- (void)startRenderer {
    if (!_displayLink) {
        VROWeakProxy *proxy = [VROWeakProxy weakProxyForObject:self];
        _displayLink = [CADisplayLink displayLinkWithTarget:proxy selector:@selector(display)];
        if ([_displayLink respondsToSelector:@selector(preferredFramesPerSecond)]) {
            _displayLink.preferredFramesPerSecond = 60;
        }
        
        [_displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
        _displayLink.paused = _paused;
    }
    
    [self updateOverlayView];
}

- (void)stopRenderer {
    [_displayLink invalidate];
    _displayLink = nil;
}

#pragma mark - ViroView

/*
 This function will asynchronously validate the given API key and notify the
 renderer if the key is invalid.
 */
- (void)validateApiKey:(NSString *)apiKey withCompletionBlock:(VROViewValidApiKeyBlock)completionBlock {
    // If the user gives us a key, then let them use the API until we successfully checked the key.
    _sceneRenderer->setSuspended(false);
    __weak typeof(self) weakSelf = self;
    
    VROApiKeyValidatorBlock validatorCompletionBlock = ^(BOOL valid) {
        VROViewGVR *strongSelf = weakSelf;
        if (!strongSelf) {
            return;
        }
        
        strongSelf->_sceneRenderer->setSuspended(!valid);
        completionBlock(valid);
        NSLog(@"[ApiKeyValidator] The key is %@!", valid ? @"valid" : @"invalid");
    };
    [self.keyValidator validateApiKey:apiKey platform:[self getPlatform] withCompletionBlock:validatorCompletionBlock];
    
  // Record keen_io metrics
  [[VROMetricsRecorder sharedClientWithViewType:@"VR" platform:@"cardboard"] recordEvent:@"renderer_init"];
}

- (void)recenterTracking {
    _sceneRenderer->recenterTracking();
}

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    _renderDelegateWrapper = std::make_shared<VRORenderDelegateiOS>(renderDelegate);
    self.renderer->setDelegate(_renderDelegateWrapper);
}

- (void)setPointOfView:(std::shared_ptr<VRONode>)node {
    self.renderer->setPointOfView(node);
}

- (void)setSceneController:(std::shared_ptr<VROSceneController>) sceneController {
    _sceneController = sceneController;
    _renderer->setSceneController(_sceneController, _driver);
}

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController
                  duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    _sceneController = sceneController;
    _renderer->setSceneController(sceneController, seconds, timingFunctionType, _driver);
}

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
}

- (std::shared_ptr<VRORenderer>)renderer {
    return _renderer;
}

- (std::shared_ptr<VROChoreographer>)choreographer {
    return _renderer->getChoreographer();
}

- (NSString *)getPlatform {
    return @"gvr";
}

- (NSString *)getHeadset {
    return [NSString stringWithUTF8String:_renderer->getInputController()->getHeadset().c_str()];
}

- (NSString *)getController {
    return [NSString stringWithUTF8String:_renderer->getInputController()->getController().c_str()];
}

- (void)setDebugHUDEnabled:(BOOL)enabled {
    self.renderer->setDebugHUDEnabled(enabled);
}

#pragma mark - Recording and Screenshots

- (void)startVideoRecording:(NSString *)filename
           saveToCameraRoll:(BOOL)saveToCamera
                 errorBlock:(VROViewRecordingErrorBlock)errorBlock {
    // no-op
}

- (void)stopVideoRecordingWithHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    // no-op
}

- (void)takeScreenshot:(NSString *)fileName
      saveToCameraRoll:(BOOL)saveToCamera
 withCompletionHandler:(VROViewWriteMediaFinishBlock)completionHandler {
    // no-op
}

@end

#pragma mark - GVROverlayViewDelegate

@implementation VROOverlayViewDelegate

- (void)didTapBackButton {
    VROViewGVR *view = self.view;
    if (view) {
        view.renderer->requestExitVR();
    }
}

- (UIViewController *)parentViewController {
    UIResponder *responder = self.view;
    while ([responder isKindOfClass:[UIView class]]) {
        responder = [responder nextResponder];
    }
    return (UIViewController *)responder;
}

- (UIViewController *)presentingViewControllerForSettingsDialog {
    return [self parentViewController];
}

- (void)didPresentSettingsDialog:(BOOL)presented {
    
}

- (void)didChangeViewerProfile {
    VROViewGVR *view = self.view;
    if (view) {
        self.view.sceneRenderer->refreshViewerProfile();
    }
}

- (void)shouldDisableIdleTimer:(BOOL)shouldDisable {
    [UIApplication sharedApplication].idleTimerDisabled = shouldDisable;
}

@end
