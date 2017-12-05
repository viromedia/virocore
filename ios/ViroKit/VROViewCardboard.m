//
//  VROViewCardboard.m
//  ViroRenderer
//
//  Created by Raj Advani on 4/28/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#import "VROViewCardboard.h"
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
#import "GVROverlayView.h"
#include "vr/gvr/capi/include/gvr_audio.h"

@interface VROOverlayViewDelegate : NSObject <GVROverlayViewDelegate>

@property (readwrite, nonatomic, weak) VROViewCardboard *view;

@end

@interface VROViewCardboard () {
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
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

@implementation VROViewCardboard

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

- (instancetype)initWithFrame:(CGRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        [self initRenderer];
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

- (void)initRenderer {
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
    _gvrAudio = std::make_shared<gvr::AudioApi>();
    _gvrAudio->Init(GVR_AUDIO_RENDERING_BINAURAL_HIGH_QUALITY);
    _driver = std::make_shared<VRODriverOpenGLiOSGVR>(self, self.context, _gvrAudio);
    _renderer = std::make_shared<VRORenderer>(std::make_shared<VROInputControllerCardboardiOS>());
    _sceneRenderer = std::make_shared<VROSceneRendererGVR>(_gvrAudio,
                                                           self.bounds.size.width * self.contentScaleFactor,
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
    NSLog(@"GVR: application did become active");
    self.paused = NO;
}

- (void)setVrMode:(BOOL)enabled {
    if (_VRModeEnabled == enabled) {
        return;
    }
    _sceneRenderer->setVRModeEnabled(enabled);
    _VRModeEnabled = enabled;
}

- (void)setPaused:(BOOL)paused {
    _paused = paused;
    if (_paused) {
        _renderer->getInputController()->onPause();
        _gvrAudio->Pause();
        _sceneRenderer->pause();
    }
    else {
        _renderer->getInputController()->onResume();
        _gvrAudio->Resume();
        _sceneRenderer->resume();
    }
    
    _displayLink.paused = (self.superview == nil || _paused);
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
    
    if (!_initialized && self.bounds.size.width > 0 && self.bounds.size.height > 0) {
        _initialized = YES;
        [self bindDrawable];
        _sceneRenderer->initGL();
    }
    
    // This check is needed for some rare cases where we're getting layoutSubviews
    // invoked mid-orientation change, resulting in 0 width or height
    if (self.bounds.size.width > 0 && self.bounds.size.height > 0) {
        UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
        _sceneRenderer->setSurfaceSize(self.bounds.size.width * self.contentScaleFactor,
                                       self.bounds.size.height * self.contentScaleFactor,
                                       orientation);
    }
}

- (void)updateOverlayView {
    // Transition view is always shown when VR mode is toggled ON.
    _overlayView.hidesTransitionView = _overlayView.hidesTransitionView || !_VRModeEnabled;
    _overlayView.hidesSettingsButton = !_VRModeEnabled;
    _overlayView.hidesAlignmentMarker = !_VRModeEnabled;
    _overlayView.hidesFullscreenButton = !_VRModeEnabled;
    _overlayView.hidesCardboardButton = _VRModeEnabled;
    
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
        VROViewCardboard *strongSelf = weakSelf;
        if (!strongSelf) {
            return;
        }
        
        strongSelf->_sceneRenderer->setSuspended(!valid);
        completionBlock(valid);
        NSLog(@"[ApiKeyValidator] The key is %@!", valid ? @"valid" : @"invalid");
    };
    [self.keyValidator validateApiKey:apiKey platform:[self getPlatform] withCompletionBlock:validatorCompletionBlock];
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
    VROViewCardboard *view = self.view;
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
    VROViewCardboard *view = self.view;
    if (view) {
        self.view.sceneRenderer->refreshViewerProfile();
    }
}

- (void)shouldDisableIdleTimer:(BOOL)shouldDisable {
    [UIApplication sharedApplication].idleTimerDisabled = shouldDisable;
}

@end
