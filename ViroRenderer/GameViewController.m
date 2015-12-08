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
#import "VROSurface.h"
#import "VROBox.h"
#import "VRONode.h"
#import "VROView.h"
#import "VROLight.h"

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getCenterLayer() {
    return centerLayer;
}

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getTopLayer() {
    return topLayer;
}

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getBottomLayer() {
    return bottomLayer;
}

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getLeftLayer() {
    return leftLayer;
}

std::shared_ptr<VROLayer> VROMomentsLayoutDelegate::getRightLayer() {
    return rightLayer;
}

@implementation GameViewController
{
    // view
    MTKView *_view;
    std::shared_ptr<VROScene> _scene;
    std::shared_ptr<VROCrossLayout> _layout;
    
    std::shared_ptr<VRONode> _rootNode;
    float angle;
    
}

- (void)setupRendererWithView:(MTKView *)view context:(VRORenderContext *)renderContext {
    [self _loadAssetsWithContext:renderContext];
}

- (void)shutdownRendererWithView:(MTKView *)view {
    
}

- (void)_loadAssetsWithContext:(VRORenderContext *)context {
    _scene = std::make_shared<VROScene>();
    _layout = std::make_shared<VROCrossLayout>(_scene);
    
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(1, 1);
    std::shared_ptr<VROBox> box = VROBox::createBox(1, 1, 1);
    
    _rootNode = std::make_shared<VRONode>(*context);
    _rootNode->setGeometry(box);
    _rootNode->setPosition({0, 0, 5});
    
    _scene->addNode(_rootNode);
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Directional);
    light->setColor({ 1.0, 1.0, 1.0, 1.0 });
    light->setDirection( { 0, 0, 1.0 });
    
    _rootNode->setLight(light);
    
    if (true) {
        return;
    }
    
    size_t dataLength;
    int width, height;
    void *data = VROImageLoadTextureDataRGBA8888("momentslogo", &dataLength, &width, &height);
    
    std::shared_ptr<VROLayer> center = std::make_shared<VROLayer>(*context);
    center->setContents(data, dataLength, width, height);
    
    std::shared_ptr<VROLayer> top = std::make_shared<VROLayer>(*context);
    top->setContents(data, dataLength, width, height);
    
    std::shared_ptr<VROLayer> bottom = std::make_shared<VROLayer>(*context);
    bottom->setContents(data, dataLength, width, height);
    
    std::shared_ptr<VROLayer> left = std::make_shared<VROLayer>(*context);
    left->setContents(data, dataLength, width, height);
    
    std::shared_ptr<VROLayer> right = std::make_shared<VROLayer>(*context);
    right->setContents(data, dataLength, width, height);
    
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
    
    center->addSublayer(labelView.vroLayer);
    
    std::shared_ptr<VROMomentsLayoutDelegate> delegate = std::make_shared<VROMomentsLayoutDelegate>(center, top, bottom, left, right);
    _layout->setDelegate(delegate);
    _layout->layout();
    
    free (data);
}

- (void)prepareNewFrameWithHeadViewMatrix:(matrix_float4x4)headViewMatrix {
    
}

- (void)renderEye:(VROEyeType)eye context:(VRORenderContext *)renderContext {
    angle += .01;
    _rootNode->setRotation({ 0, angle, 0});
    
    _scene->render(*renderContext);
}

- (void)renderViewDidChangeSize:(CGSize)size context:(VRORenderContext *)context {

}

@end

