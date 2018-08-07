//
//  VROTestViewController.m
//  ViroSample
//
//  Created by Raj Advani on 10/3/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROTestViewController.h"
#import "VRORenderer.h"

enum class VROTestSceneType {
    VR,
    AR,
    Scene,
};

static const VROTestSceneType kTestType = VROTestSceneType::AR;
static const VRORendererTestType kRendererTest = VRORendererTestType::ARImageTracking;
static const bool kSceneCheckeredBackground = NO;

@interface VROTestViewController ()

@end

@implementation VROTestViewController

- (void)loadView {
    VRORendererConfiguration config;
    config.enableHDR = YES;
    config.enablePBR = YES;
    config.enableBloom = YES;
    
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
                                            worldAlignment:VROWorldAlignment::Gravity];
        view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        view.renderDelegate = self.renderDelegate;
        view.suspended = NO;
        
        self.renderDelegate.view = view;
        self.renderDelegate.test = kRendererTest;
        self.view = view;

        if (kRendererTest == VRORendererTestType::ARImageTracking) {
            std::shared_ptr<VROARSession> session = [view getARSession];
            session->setNumberOfTrackedImages(1);
        }
        
        [self testVideoRecording];
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
    
    NSLog(@"[VROTestViewController] started video recording");
    
    NSString *filename = [NSString stringWithFormat:@"testvideo%d", rand];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [arView startVideoRecording:filename saveToCameraRoll:YES errorBlock:nil];
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        NSLog(@"[VROTestViewController] stopped video recording");
        [arView stopVideoRecordingWithHandler:^(BOOL success, NSURL *url, NSInteger errorCode) {
            if (url) {
                [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
            }
        }];
    });
    
    NSString *filename2 = [NSString stringWithFormat:@"testvideo%d", rand];
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(8 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [arView startVideoRecording:filename2 saveToCameraRoll:YES errorBlock:nil];
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC), dispatch_get_main_queue(), ^{
        NSLog(@"[VROTestViewController] stopped video recording");
        [arView stopVideoRecordingWithHandler:^(BOOL success, NSURL *url, NSInteger errorCode) {
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
        [arView takeScreenshot:filename saveToCameraRoll:YES withCompletionHandler:^(BOOL success, NSURL *url, NSInteger errorCode) {
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
