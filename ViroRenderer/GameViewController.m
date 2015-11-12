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
#import "VROView.h"

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
    void *data = VROImageLoadTextureDataRGBA8888("momentslogo", &dataLength, &width, &height);
    
    std::shared_ptr<VROLayer> layerA = std::make_shared<VROLayer>(*context);
    layerA->setFrame(VRORectMake(-1, -1, 10, 2.0, 2.0));
    layerA->setBackgroundColor({ 1.0, 1.0, 1.0, 1.0 });
    layerA->setContents(data, dataLength, width, height);
    
    VROView *labelView = [[VROView alloc] initWithFrame:CGRectMake(0, 0, 100, 10) context:context];
    labelView.vroLayer->setFrame(VRORectMake(0, 1.9, 2.0, 0.2));
    
    [labelView setBackgroundColor:[UIColor clearColor]];
    
    UILabel *label = [[UILabel alloc] initWithFrame:labelView.bounds];
    [label setText:@"Moments"];
    [label setBackgroundColor:[UIColor clearColor]];
    [label setTextAlignment:NSTextAlignmentCenter];
    [label setTextColor:[UIColor whiteColor]];
    [label setFont:[UIFont systemFontOfSize:14]];
    
    [labelView addSubview:label];
    [labelView update];
    
    _scene->addLayer(layerA);
    layerA->addSublayer(labelView.vroLayer);
    
    free (data);
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        layerA->setPosition({0, 0, 1});
        VROAnimation::setAnimationDuration(2);
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

