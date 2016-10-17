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
#import "VRORenderLoopCardboard.h"
#import "VROSceneRendererCardboardMetal.h"
#import "VROSceneRendererCardboardOpenGL.h"
#import "VROFieldOfView.h"
#import "VROViewport.h"

@interface VROViewCardboard () {
    std::shared_ptr<VRORenderer> _renderer;
    VRORenderLoopCardboard *_renderLoop;
}

@property (readwrite, nonatomic) std::shared_ptr<VRORenderer> renderer;
@property (readwrite, nonatomic) VROSceneRendererCardboard *sceneRenderer;
@property (readwrite, nonatomic) VROFieldOfView fov;
@property (readwrite, nonatomic) VROViewport viewport;

@end

@implementation VROViewCardboard

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
    self.delegate = self;
    
    // Do not allow the display to go into sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    self.renderer = std::make_shared<VRORenderer>();
    self.sceneRenderer = new VROSceneRendererCardboardOpenGL(self.context, self.renderer);

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self
                                                                                    action:@selector(handleTap:)];
    [self addGestureRecognizer:tapRecognizer];
    
    // TODO Bug in Cardboard prevents [headTransform fieldOfViewForEye:] from working
    self.fov = { 30.63, 40, 40, 34.178 };
}

- (void) setVrMode:(BOOL)enabled {
    self.vrModeEnabled = enabled;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
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

#pragma mark - Settings

- (void)orientationDidChange:(NSNotification *)notification {
  // If the orientation changes, set the frame of the view to the asme as the screen.
  self.frame = CGRectMake(0, 0, [[UIScreen mainScreen] bounds].size.width, [[UIScreen mainScreen] bounds].size.height);
}

- (void)setRenderDelegate:(id<VRORenderDelegate>)renderDelegate {
    self.renderer->setDelegate(renderDelegate);
}

#pragma mark - Camera

- (void)setPosition:(VROVector3f)position {
    self.renderer->setPosition(position);
}

- (void)setBaseRotation:(VROQuaternion)rotation {
    self.renderer->setBaseRotation(rotation);
}

- (void)setCameraRotationType:(VROCameraRotationType)type {
    self.renderer->setCameraRotationType(type);
}

- (void)setOrbitFocalPoint:(VROVector3f)focalPt {
    self.renderer->setOrbitFocalPoint(focalPt);
}

- (float)worldPerScreenAtDepth:(float)distance {
    return self.renderer->getWorldPerScreen(distance, self.fov, self.viewport);
}

- (void)layoutSubviews {
    [super layoutSubviews];
    _renderer->updateRenderViewSize(self.bounds.size);
}

#pragma mark - Reticle

- (void)handleTap:(UIGestureRecognizer *)gestureRecognizer {
    _renderer->handleTap();
}

- (VROScreenUIView *)HUD {
    return _renderer->getHUD();
}

#pragma mark - Scene Loading

- (VROSceneController *)sceneController {
    return _renderer->getSceneController();
}

- (void)setSceneController:(VROSceneController *)sceneController {
    self.sceneRenderer->setSceneController(sceneController);
}

- (void)setSceneController:(VROSceneController *)sceneController animated:(BOOL)animated {
    self.sceneRenderer->setSceneController(sceneController, animated);
}

- (void)setSceneController:(VROSceneController *)sceneController duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    
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
            _renderer->handleTap();
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
    
    self.sceneRenderer->prepareFrame(headTransform);
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
