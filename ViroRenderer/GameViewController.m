//
//  GameViewController.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "GameViewController.h"
#import "SharedStructures.h"
#import "VROAnimation.h"
#import "VRORenderContextMetal.h"
#import "VROScene.h"
#import "VROLayer.h"
#import "VROMath.h"
#import "VROImageUtil.h"

@implementation GameViewController
{
    // view
    MTKView *_view;
    
    VROScene *_scene;
}

- (void)setupRendererWithView:(MTKView *)view context:(VRORenderContext *)renderContext {
    [self _loadAssetsWithContext:renderContext];
}

- (void)shutdownRendererWithView:(MTKView *)view {
    
}

- (void)_loadAssetsWithContext:(VRORenderContext *)context {
    _scene = new VROScene();
    
    size_t dataLength;
    int width, height;
    void *data = VROImageLoadTextureDataRGBA8888("boba", &dataLength, &width, &height);
    
    std::shared_ptr<VROLayer> layerA = std::make_shared<VROLayer>(*context);
    layerA->setFrame(VRORectMake(-1.0, 0, 2, 2.0, 2.0));
    layerA->setBackgroundColor({ 1.0, 0.0, 0.0, 1.0 });
    layerA->setContents(data, dataLength, width, height);
    
    std::shared_ptr<VROLayer> layerB = std::make_shared<VROLayer>(*context);
    layerB->setFrame(VRORectMake(1.0, 1.0, 1.0, 1.0));
    layerB->setBackgroundColor({ 0.0, 0.0, 1.0, 1.0 });
    layerB->setContents(data, dataLength, width, height);

    std::shared_ptr<VROLayer> layerC = std::make_shared<VROLayer>(*context);
    layerC->setFrame(VRORectMake(0.0, 0.0, 0.5, 0.5));
    layerC->setBackgroundColor({ 0.0, 1.0, 0.0, 1.0 });
    layerC->setContents(data, dataLength, width, height);
    
    _scene->addLayer(layerA);
    layerA->addSublayer(layerB);
    layerB->addSublayer(layerC);
    
    free (data);
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        layerA->setPosition({0.5, 1.0, 3.0});
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROAnimation::begin();
        VROAnimation::setAnimationDuration(1.0);
        layerA->setPosition({0.5, -1.0, 4.0});
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROAnimation::begin();
        VROAnimation::setAnimationDuration(2.0);
        layerA->setPosition({-0.5, -1.0, 3.0});
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        layerA->setPosition({0, 0, 2.0});
    });
}

- (void)prepareNewFrameWithHeadViewMatrix:(matrix_float4x4)headViewMatrix {
    
}

- (void)renderEye:(VROEyeType)eye context:(VRORenderContext *)renderContext {
    _scene->render(*renderContext);
}

- (void)renderViewDidChangeSize:(CGSize)size context:(VRORenderContext *)context {

}

@end

