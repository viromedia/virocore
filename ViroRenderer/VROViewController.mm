//
//  VROViewController.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/23/15.
//  Copyright © 2015 Viro Media. All rights reserved.
//

#import "VROViewController.h"

#import "VRODevice.h"
#import "VRODistortion.h"
#import "VRODistortionRenderer.h"
#import "VROEye.h"
#import "VROFieldOfView.h"
#import "VROViewport.h"
#import "VROScreen.h"
#import "VROHeadTracker.h"
#import "VROMagnetSensor.h"
#import "VRORenderContextMetal.h"
#import "VROAnimation.h"
#import "VROImageUtil.h"

@implementation VROViewController

@dynamic view;

- (BOOL)prefersStatusBarHidden {
    return YES;
}

@end
