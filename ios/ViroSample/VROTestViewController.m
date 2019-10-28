//
//  VROTestViewController.m
//  ViroSample
//
//  Created by Raj Advani on 10/3/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#import "VROTestViewController.h"
#import "VRORenderer.h"

enum class VROTestSceneType {
    VR,
    AR,
    Scene,
};

static const VROTestSceneType kTestType = VROTestSceneType::AR;
static const VRORendererTestType kRendererTest = VRORendererTestType::FBX;
static const bool kSceneCheckeredBackground = NO;

@interface VROTestViewController ()

@end

@implementation VROTestViewController

- (void)loadView {
    VRORendererConfiguration config;
    config.enableHDR = YES;
    config.enablePBR = YES;
    config.enableBloom = YES;
    config.enableMultisampling = NO;
    
    if (kTestType == VROTestSceneType::VR) {
        VROViewGVR *view = [[VROViewGVR alloc] initWithFrame:[UIScreen mainScreen].bounds
                                                      config:config];
        view.testingMode = YES;
        view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        view.renderDelegate = self.renderDelegate;
        
        self.renderDelegate.view = view;
        self.renderDelegate.test = kRendererTest;
        self.view = view;
    }
    else if (kTestType == VROTestSceneType::AR) {
        VROViewAR *view = [[VROViewAR alloc] initWithFrame:[UIScreen mainScreen].bounds
                                                    config:config
                                                   context:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]
                                            worldAlignment:VROWorldAlignment::Gravity
                                              trackingType:VROTrackingType::DOF6];
        view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        view.renderDelegate = self.renderDelegate;
        view.suspended = NO;
        
        self.renderDelegate.view = view;
        self.renderDelegate.test = kRendererTest;
        self.view = view;
        
        //[self testVideoRecording];
        //[self testScreenshot];
    }
    else {
        CGRect screenBounds = [UIScreen mainScreen].bounds;
        CGRect viroBounds;
        if (kSceneCheckeredBackground) {
            viroBounds = CGRectInset(screenBounds, 50, 50);
        } else {
            viroBounds = screenBounds;
        }
        
        VROViewScene *view = [[VROViewScene alloc] initWithFrame:viroBounds
                                                          config:config
                                                         context:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]];
        view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        view.renderDelegate = self.renderDelegate;
        view.suspended = NO;
        view.backgroundColor = [UIColor colorWithRed:0 green:0 blue:0.0 alpha:0.0];
        
        self.renderDelegate.view = view;
        self.renderDelegate.test = kRendererTest;
        
        if (kSceneCheckeredBackground) {
            UIView *backingView = [[UIView alloc] initWithFrame:screenBounds];
            backingView.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
            backingView.backgroundColor = [UIColor colorWithPatternImage:[UIImage imageNamed:@"check"]];
            [backingView addSubview:view];
            
            self.view = backingView;
        } else {
            self.view = view;
        }
    }
}

- (void)testVideoRecording {
    VROViewAR *arView = (VROViewAR *)self.view;
    int rand = arc4random_uniform(1000);
    
    NSString *filename = [NSString stringWithFormat:@"testvideo%d", rand];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        NSLog(@"[Recording] Starting video recording...");

        // test record with a watermark
        UIImage *image = [UIImage imageNamed:@"app_logo_viro.png"];
        [arView startVideoRecording:filename withWatermark:image withFrame:CGRectMake(200, 500, 150, 100) saveToCameraRoll:YES errorBlock:^(NSInteger errorCode) {
            NSLog(@"[Recording] Failed video recording with error code %d", (int) errorCode);
        }];
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        NSLog(@"[Recording] Stopping video recording...");
        [arView stopVideoRecordingWithHandler:^(BOOL success, NSURL *url, NSURL *gifUrl, NSInteger errorCode) {
            NSLog(@"[Recording] Finished video recording with success %d, error code %d", success, (int) errorCode);
            if (url) {
                [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
            }
        }];
    });
    
    NSString *filename2 = [NSString stringWithFormat:@"testvideo%d", rand];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(20 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        NSLog(@"[Recording] Starting video recording...");

        [arView startVideoRecording:filename2 saveToCameraRoll:YES errorBlock:nil];
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 25 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        NSLog(@"[Recording] Stopping video recording...");
        [arView stopVideoRecordingWithHandler:^(BOOL success, NSURL *url, NSURL *gifUrl, NSInteger errorCode) {
            NSLog(@"[Recording] Finished video recording with success %d, error code %d", success, (int) errorCode);
            if (url) {
                [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
            }
        }];
    });
}

- (void)testScreenshot {
    VROViewAR *arView = (VROViewAR *)self.view;
    int rand = arc4random_uniform(1000);
    
    NSString *filename = [NSString stringWithFormat:@"testimage%d", rand];
    NSLog(@"[VROSample] taking screenshot in 5 seconds");
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        [arView takeScreenshot:filename saveToCameraRoll:YES withCompletionHandler:^(BOOL success, NSURL *url, NSURL *gifUrl, NSInteger errorCode) {
            if (url) {
                [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
            }
        }];
    });
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (UIInterfaceOrientationMask)supportedInterfaceOrientations {
    if (kTestType == VROTestSceneType::VR) {
        // GVR only supports landscape right orientation for inserting the phone in the viewer.
        return UIInterfaceOrientationMaskLandscapeRight;
    }
    else {
        return UIInterfaceOrientationMaskAll;
    }
}

@end
