//
//  VROView.m
//  ViroRenderer
//
//  Created by Raj Advani on 12/15/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import "VROView.h"
#import "VROTime.h"
#import "VRODevice.h"
#import "VRODistortion.h"
#import "VRODistortionRenderer.h"
#import "VROEye.h"
#import "VROFieldOfView.h"
#import "VROViewport.h"
#import "VROHeadTracker.h"
#import "VROTransaction.h"
#import "VROImageUtil.h"
#import "VROProjector.h"
#import "VROAllocationTracker.h"
#import "VROScene.h"
#import "VROSceneController.h"
#import "VROLog.h"
#import "VROCameraMutable.h"
#import "VRODriverMetal.h"
#import "VRORenderer.h"

@interface VROView () {
  
}

@property (readwrite, nonatomic) std::shared_ptr<VRODriverMetal> driver;
@property (readwrite, nonatomic) std::shared_ptr<VRORenderer> renderer;

@end

@implementation VROView

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
    // Do not allow the display to go into sleep
    [UIApplication sharedApplication].idleTimerDisabled = YES;
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(orientationDidChange:)
                                                 name:UIApplicationDidChangeStatusBarOrientationNotification
                                               object:nil];
    self.renderer = std::make_shared<VRORenderer>();
    self.driver = std::make_shared<VRODriverMetal>(self.renderer, self);

    UITapGestureRecognizer *tapRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:self
                                                                                    action:@selector(handleTap:)];
    [self addGestureRecognizer:tapRecognizer];
    
    UIView *renderingView = self.driver->getRenderingView();
    [renderingView setFrame:self.bounds];
    [renderingView setAutoresizingMask:UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight];
    
    [self addSubview:renderingView];
    [self sendSubviewToBack:renderingView];
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

#pragma mark - Settings

- (void)orientationDidChange:(NSNotification *)notification {
    self.driver->onOrientationChange([UIApplication sharedApplication].statusBarOrientation);
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

- (float)worldPerScreenAtDepth:(float)distance {
    return self.renderer->getWorldPerScreen(distance,
                                            self.driver->getFOV(VROEyeType::Left),
                                            self.driver->getViewport(VROEyeType::Left));
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

@end
