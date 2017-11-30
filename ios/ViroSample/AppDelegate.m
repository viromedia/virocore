//
//  AppDelegate.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "AppDelegate.h"
#import "VROTestViewController.h"
#import "VROTestRenderDelegate.h"

@interface AppDelegate () <UINavigationControllerDelegate>
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    VROTestViewController *viewController = [[VROTestViewController alloc] init];
    viewController.renderDelegate = [[VROTestRenderDelegate alloc] init];
    
    UINavigationController *navigationController = [[UINavigationController alloc]
                                                    initWithRootViewController:viewController];
    navigationController.navigationBarHidden = YES;
    navigationController.delegate = self;

    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.rootViewController = navigationController;
    [self.window makeKeyAndVisible];
    return YES;
}

- (UIInterfaceOrientationMask)navigationControllerSupportedInterfaceOrientations:(UINavigationController *)navigationController {
    return [navigationController.topViewController supportedInterfaceOrientations];
}

@end
