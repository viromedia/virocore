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

- (void)willRenderEye:(VROEyeType)eye context:(const VRORenderContext *)renderContext {}
- (void)didRenderEye:(VROEyeType)eye context:(const VRORenderContext *)renderContext {}
- (void)renderViewDidChangeSize:(CGSize)size context:(VRORenderContext *)context {}
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
    _harness = std::make_shared<VRORendererTestHarness>(self.view.frameSynchronizer, driver);
    std::shared_ptr<VRORendererTest> test = _harness->loadTest(VRORendererTestType::VideoSphere);
    
    self.view.sceneController = test->getSceneController();
    if (test->getPointOfView()) {
        [self.view setPointOfView:test->getPointOfView()];
    }
}

@end

