//
//  VROViewControllerGVR.m
//  ViroKit
//
//  Created by Raj Advani on 11/30/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROViewControllerGVR.h"
#import "VROViewGVR.h"

@implementation VROViewControllerGVR

- (VROViewGVR *)rendererView {
    return (VROViewGVR *)self.view;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)loadView {
    self.view = [[VROViewGVR alloc] initWithFrame:[UIScreen mainScreen].bounds];
}

- (BOOL)shouldAutorotate {
    return NO;
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
    return UIInterfaceOrientationMaskLandscapeRight;
}

- (void)viewWillAppear:(BOOL)animated {
    [super viewWillAppear:animated];
    self.rendererView.paused = NO;
    
    // Force to landscape right; this appears to be the only reliable way to do this
    // When we tried doing this any other way we run into a GVR bug where the viewport
    // gets cut in half. Specifically this occurs when the device begins in portrait
    // mode.
    
    // Note this fix sometimes creates an issue where the height of the *superview*
    // becomes zero. We check for (and fix) this in VROViewGVR.layoutSubviews().
    [[UIDevice currentDevice] setValue:[NSNumber numberWithInteger: UIInterfaceOrientationLandscapeRight]
                                forKey:@"orientation"];
}

- (void)viewWillDisappear:(BOOL)animated {
    [super viewWillDisappear:animated];
    self.rendererView.paused = YES;
    
    // Since our orientation is fixed to landscape right in modal state, upon returning from the modal
    // state, reset the UI orientation to the device's orientation.
    if (self.isModal) {
        [UIViewController attemptRotationToDeviceOrientation];
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // We need device orientation change notifications to work.
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
    });
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(didChangeOrientation:)
                                                 name:UIDeviceOrientationDidChangeNotification
                                               object:nil];
}

- (void)didChangeOrientation:(NSNotification *)notification {
    // Request a layout change on the render view since iOS does not call layoutSubviews on 180-degree
    // orientation changes.
    [self.view setNeedsLayout];
}

// Returns YES if this view controller is being presented. */
- (BOOL)isModal {
    if ([self presentingViewController]) {
        return YES;
    }
    if ([[[self navigationController] presentingViewController] presentedViewController] ==
        [self navigationController]) {
        return YES;
    }
    if ([[[self tabBarController] presentingViewController]
         isKindOfClass:[UITabBarController class]]) {
        return YES;
    }
    return NO;
}

@end
