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

static const VRORendererTestType kRendererTest = VRORendererTestType::FBX;

@interface VROTestViewController ()

@end

@implementation VROTestViewController

- (void)loadView {
    VRORendererConfiguration config;
    config.enableHDR = NO;
    config.enablePBR = NO;
    config.enableBloom = NO;
    
    VROViewScene *view = [[VROViewScene alloc] initWithFrame:NSMakeRect(20, 20, 600, 600)
                                                      config:config
                                                shareContext:nil];
    view.renderDelegate = self.renderDelegate;
    
    self.renderDelegate.view = view;
    self.renderDelegate.test = kRendererTest;
    
    self.view = view;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
}

@end
