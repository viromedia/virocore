//
//  VROTestViewController.m
//  ViroSample
//
//  Created by Raj Advani on 10/3/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#import "VROTestViewController.h"

// Set to NO to test using an AR view
static const BOOL kTestVR = NO;

@interface VROTestViewController ()

@end

@implementation VROTestViewController

- (void)loadView {
    if (kTestVR) {
        VROViewCardboard *view = [[VROViewCardboard alloc] initWithFrame:[UIScreen mainScreen].bounds];
        view.testingMode = YES;
        view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        view.renderDelegate = self.renderDelegate;
        [view awakeFromNib];
        
        self.renderDelegate.view = view;
        self.view = view;
    }
    else {
        VROViewAR *view = [[VROViewAR alloc] initWithFrame:[UIScreen mainScreen].bounds
                                                   context:[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3]];
        view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
        view.renderDelegate = self.renderDelegate;
        
        self.renderDelegate.view = view;
        self.view = view;
    }
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

@end
