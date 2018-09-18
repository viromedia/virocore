//
//  VROSample.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "VROTestRenderDelegate.h"

@interface VROTestRenderDelegate ()
@property (readwrite, nonatomic) std::shared_ptr<VRORendererTestHarness> harness;
@end

@implementation VROTestRenderDelegate

- (void)userDidRequestExitVR {}

- (void)nextSceneAfterDelay {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self nextScene:nullptr];
        [self nextSceneAfterDelay];
    });
}

- (IBAction)nextScene:(id)sender {
 
}

- (void)setupRendererWithDriver:(std::shared_ptr<VRODriver>)driver {
    _harness = std::make_shared<VRORendererTestHarness>(self.view.renderer, self.view.frameSynchronizer, driver, self.view);
    std::shared_ptr<VRORendererTest> test = _harness->loadTest(self.test);
    
    self.view.sceneController = test->getSceneController();
    if (test->getPointOfView()) {
        [self.view setPointOfView:test->getPointOfView()];
    }
}

@end

