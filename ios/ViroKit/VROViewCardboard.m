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
#include "vr/gvr/capi/include/gvr.h"
#include "vr/gvr/capi/include/gvr_audio.h"
#include "vr/gvr/capi/include/gvr_types.h"

@interface VROViewCardboard () {
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
    std::shared_ptr<VRODriverOpenGL> _driver;
    std::shared_ptr<gvr::AudioApi> _gvrAudio;
    CADisplayLink *_displayLink;
    gvr_context *_gvr;
    GVROverlayView *_gvrOverlay;
}

@property (readwrite, nonatomic) std::shared_ptr<VRORenderer> renderer;
@property (readwrite, nonatomic) std::shared_ptr<VROSceneRendererGVR> sceneRenderer;
@property (readwrite, nonatomic) id <VROApiKeyValidator> keyValidator;

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
    gvr_destroy(&_gvr);
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
    
    /*
     Setup the GLKView.
     */
    self.enableSetNeedsDisplay = NO;
    self.drawableColorFormat = GLKViewDrawableColorFormatSRGBA8888;
    self.drawableStencilFormat = GLKViewDrawableStencilFormat8;
    self.drawableDepthFormat = GLKViewDrawableDepthFormat16;
    self.drawableMultisample = GLKViewDrawableMultisample4X;
    
    [EAGLContext setCurrentContext:self.context];
    [self bindDrawable];
    _gvr = gvr_create();
    
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
    _sceneRenderer = std::make_shared<VROSceneRendererGVR>(_gvr, _gvrAudio,
                                                           self.bounds.size.width * self.contentScaleFactor,
                                                           self.bounds.size.height * self.contentScaleFactor,
                                                           [[UIApplication sharedApplication] statusBarOrientation],
                                                           self.contentScaleFactor, _renderer, _driver);
    _sceneRenderer->initGL();
    [self setVrMode:true];
    
    /*
     Set up the Audio Session properly for recording and playing back audio. We need
     to do this *AFTER* we init _gvrAudio, because it resets some setting, else audio
     recording won't work.
     */
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayAndRecord
             withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker
                   error:nil];
    
    self.keyValidator = [[VROApiKeyValidatorDynamo alloc] init];
    
    /*
     Add a tap gesture to handle viewer trigger action.
     */
    UITapGestureRecognizer *tapGesture = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(didTapGLView:)];
    [self addGestureRecognizer:tapGesture];
}

- (void)applicationWillResignActive:(NSNotification *)notification {
    _renderer->getInputController()->onPause();
    gvr_pause_tracking(_gvr);
    _gvrAudio->Pause();
}

- (void)applicationDidBecomeActive:(NSNotification *)notification {
    _renderer->getInputController()->onResume();
    gvr_refresh_viewer_profile(_gvr);
    gvr_resume_tracking(_gvr);
    _gvrAudio->Resume();
}

- (void)setDebugHUDEnabled:(BOOL)enabled {
    self.renderer->setDebugHUDEnabled(enabled);
}

- (void)setVrMode:(BOOL)enabled {
    // Pull down the GVR overlay if it's already displayed
    if (_gvrOverlay && _gvrOverlay.superview) {
        [_gvrOverlay removeFromSuperview];
    }
    
    // If turning VR on display the GVR overlay view for inserting the device into the headset
    if (enabled) {
        _gvrOverlay = [[GVROverlayView alloc] initWithFrame:self.bounds];
        _gvrOverlay.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        _gvrOverlay.delegate = self;
        [self addSubview:_gvrOverlay];
    }
    _sceneRenderer->setVRModeEnabled(enabled);
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
    gvr_recenter_tracking(_gvr);
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

#pragma mark - Settings

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    _renderDelegateWrapper = std::make_shared<VRORenderDelegateiOS>(renderDelegate);
    self.renderer->setDelegate(_renderDelegateWrapper);
}

#pragma mark - Camera

- (void)setPointOfView:(std::shared_ptr<VRONode>)node {
    self.renderer->setPointOfView(node);
}

- (void)layoutSubviews {
    [super layoutSubviews];
    _sceneRenderer->setSurfaceSize(self.bounds.size.width * self.contentScaleFactor,
                                   self.bounds.size.height * self.contentScaleFactor,
                                   [[UIApplication sharedApplication] statusBarOrientation]);
}

#pragma mark - Scene Loading

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

#pragma mark - Rendering

- (void)drawRect:(CGRect)rect {
    @autoreleasepool {
        _sceneRenderer->onDrawFrame();
    }
}

#pragma mark - Frame Listeners

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
}

#pragma mark - GVROverlayViewDelegate

- (void)didTapGLView:(id)sender {
    std::shared_ptr<VROInputControllerBase> baseController = _renderer->getInputController();
    std::shared_ptr<VROInputControllerCardboardiOS> cardboardController = std::dynamic_pointer_cast<VROInputControllerCardboardiOS>(baseController);
    cardboardController->onScreenClicked();
}

- (void)didTapBackButton {
    _renderer->requestExitVR();
}

- (UIViewController *)parentViewController {
    UIResponder *responder = self;
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
    gvr_refresh_viewer_profile(_gvr);
}

- (void)shouldDisableIdleTimer:(BOOL)shouldDisable {
    [UIApplication sharedApplication].idleTimerDisabled = shouldDisable;
}

@end
