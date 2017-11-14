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
#import "VROPlatformUtil.h"
#import "VRORenderLoopCardboard.h"
#import "VROSceneRendererCardboardMetal.h"
#import "VROSceneRendererCardboardOpenGL.h"
#import "VROFieldOfView.h"
#import "VROViewport.h"
#import "VROReticle.h"
#import "VROApiKeyValidatorDynamo.h"
#import "VROAllocationTracker.h"
#import "VRORenderDelegateiOS.h"
#import "VROInputControllerCardboardiOS.h"

@interface VROViewCardboard () {
    std::shared_ptr<VRORenderer> _renderer;
    VRORenderLoopCardboard *_renderLoop;
    std::shared_ptr<VROSceneController> _sceneController;
    std::shared_ptr<VRORenderDelegateiOS> _renderDelegateWrapper;
}

@property (readwrite, nonatomic) std::shared_ptr<VRORenderer> renderer;
@property (readwrite, nonatomic) std::shared_ptr<VROSceneRendererCardboard> sceneRenderer;
@property (readwrite, nonatomic) VROViewport viewport;
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
}

- (void)awakeFromNib {
    [super awakeFromNib];
    
    if (self.testingMode) {
        self.vrModeEnabled = YES;
        self.sceneRenderer->setSuspended(false);
    }
}

- (void)initRenderer {
    VROPlatformSetType(VROPlatformType::iOSCardboard);
    self.delegate = self;
    self.keyValidator = [[VROApiKeyValidatorDynamo alloc] init];
  
    // Do not allow the display to go into sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    VROThreadRestricted::setThread(VROThreadName::Renderer);
    self.renderer = std::make_shared<VRORenderer>(std::make_shared<VROInputControllerCardboardiOS>());
    self.sceneRenderer = std::make_shared<VROSceneRendererCardboardOpenGL>(self.context, self.renderer);
}

- (void)setDebugHUDEnabled:(BOOL)enabled {
    self.renderer->setDebugHUDEnabled(enabled);
}

- (void)setVrMode:(BOOL)enabled {
    self.vrModeEnabled = enabled;
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

- (void)willMoveToWindow:(UIWindow *)newWindow {
    if (newWindow) {
        /*
         Render loop target ensures ensures that the render loop doesn't hold a
         strong reference to this view (prevents a cycle).
         */
        VRORenderLoopTarget *target = [[VRORenderLoopTarget alloc] init];
        target.cardboardView = self; // weak reference
        
        _renderLoop = [[VRORenderLoopCardboard alloc] initWithRenderTarget:target
                                                                  selector:@selector(render)];
    }
    else {
        [_renderLoop invalidate];
        _renderLoop = nil;
    }
}

/*
 This function will asynchronously validate the given API key and notify the
 renderer if the key is invalid.
 */
- (void)validateApiKey:(NSString *)apiKey withCompletionBlock:(VROViewValidApiKeyBlock)completionBlock {
    // If the user gives us a key, then let them use the API until we successfully checked the key.
    self.sceneRenderer->setSuspended(false);
  
    __weak typeof(self) weakSelf = self;
  
    VROApiKeyValidatorBlock validatorCompletionBlock = ^(BOOL valid) {
        typeof(weakSelf) strongSelf = weakSelf;
        if (!strongSelf) {
          return;
        }
      
        strongSelf.sceneRenderer->setSuspended(!valid);
        completionBlock(valid);
      
        NSLog(@"[ApiKeyValidator] The key is %@!", valid ? @"valid" : @"invalid");
    };
    [self.keyValidator validateApiKey:apiKey platform:[self getPlatform] withCompletionBlock:validatorCompletionBlock];
}

- (void)recenterTracking {
    _sceneRenderer->recenterTracking();
}

#pragma mark - Recording (Video/Image)

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
    _renderer->updateRenderViewSize(self.bounds.size.width, self.bounds.size.height);
}

#pragma mark - Scene Loading

- (void)setSceneController:(std::shared_ptr<VROSceneController>) sceneController {
    _sceneController = sceneController;
    self.sceneRenderer->setSceneController(_sceneController);
}

- (void)setSceneController:(std::shared_ptr<VROSceneController>)sceneController
                  duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    _sceneController = sceneController;
    self.sceneRenderer->setSceneController(sceneController, seconds, timingFunctionType);
}

#pragma mark - Frame Listeners

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer->getFrameSynchronizer();
}

#pragma mark - Cardboard View Delegate

- (void)cardboardView:(GVRCardboardView *)cardboardView didFireEvent:(GVRUserEvent)event {
    if (event == kGVRUserEventTrigger) {
        dispatch_async(dispatch_get_main_queue(), ^{
            std::shared_ptr<VROInputControllerBase> baseController =  _renderer->getInputController();
            std::shared_ptr<VROInputControllerCardboardiOS> cardboardController
                        = std::dynamic_pointer_cast<VROInputControllerCardboardiOS>(baseController);
            cardboardController->onScreenClicked();
        });
    } else if (event == kGVRUserEventBackButton) {
        dispatch_async(dispatch_get_main_queue(), ^{
            _renderer->requestExitVR();
        });
    }
}

- (void)cardboardView:(GVRCardboardView *)cardboardView
     willStartDrawing:(GVRHeadTransform *)headTransform {
    
    self.sceneRenderer->initRenderer(headTransform);
}

- (void)cardboardView:(GVRCardboardView *)cardboardView
     prepareDrawFrame:(GVRHeadTransform *)headTransform {
    
    CGRect rect = [headTransform viewportForEye:kGVRLeftEye];
    self.viewport = { (int)rect.origin.x, (int)rect.origin.y, (int)rect.size.width, (int)rect.size.height };
   
    GVRFieldOfView gvrFOV = [headTransform fieldOfViewForEye:kGVRLeftEye];
    VROFieldOfView fov = { (float)gvrFOV.left, (float)gvrFOV.right, (float)gvrFOV.bottom, (float)gvrFOV.top };
    
    self.sceneRenderer->prepareFrame(self.viewport, fov, headTransform);
    ALLOCATION_TRACKER_PRINT();
}

- (void)cardboardView:(GVRCardboardView *)cardboardView
              drawEye:(GVREye)eye
    withHeadTransform:(GVRHeadTransform *)headTransform {
    
    self.sceneRenderer->renderEye(eye, headTransform);
    if (eye == kGVRRightEye || eye == kGVRCenterEye) {
        self.sceneRenderer->endFrame();
    }
}

@end
