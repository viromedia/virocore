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
    self.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
    self.vrModeEnabled = YES;
    
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
    _renderer->setSceneController(sceneController);
}

- (void)setSceneController:(VROSceneController *)sceneController animated:(BOOL)animated {
    _renderer->setSceneController(sceneController, animated);
}

- (void)setSceneController:(VROSceneController *)sceneController duration:(float)seconds
            timingFunction:(VROTimingFunctionType)timingFunctionType {
    
    _renderer->setSceneController(sceneController, seconds, timingFunctionType);
}

#pragma mark - Frame Listeners

- (std::shared_ptr<VROFrameSynchronizer>)frameSynchronizer {
    return _renderer;
}

#pragma mark - Cardboard View Delegate

- (void)cardboardView:(GCSCardboardView *)cardboardView didFireEvent:(GCSUserEvent)event {
    if (event == kGCSUserEventTrigger) {
        _renderer->handleTap();
    }
}

- (void)cardboardView:(GCSCardboardView *)cardboardView
     willStartDrawing:(GCSHeadTransform *)headTransform {
    
    
    self.sceneRenderer->initRenderer(headTransform);
}

- (void)cardboardView:(GCSCardboardView *)cardboardView
     prepareDrawFrame:(GCSHeadTransform *)headTransform {
    
    CGRect rect = [headTransform viewportForEye:kGCSLeftEye];
    self.viewport = { (int)rect.origin.x, (int)rect.origin.y, (int)rect.size.width, (int)rect.size.height };
    
    self.sceneRenderer->prepareFrame(headTransform);
}

- (void)cardboardView:(GCSCardboardView *)cardboardView
              drawEye:(GCSEye)eye
    withHeadTransform:(GCSHeadTransform *)headTransform {
    
    self.sceneRenderer->renderEye(eye, headTransform);
    if (eye == kGCSRightEye || eye == kGCSCenterEye) {
        self.sceneRenderer->endFrame();
    }
}

@end
