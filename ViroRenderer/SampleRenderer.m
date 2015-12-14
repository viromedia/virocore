//
//  GameViewController.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "SampleRenderer.h"

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

@implementation SampleRenderer {
    MTKView *_view;
    std::shared_ptr<VROScene> _scene;
    std::shared_ptr<VROCrossLayout> _layout;
    
    std::shared_ptr<VRONode> _rootNode;
    std::shared_ptr<VRONode> _boxNode;
    float angle;
}

- (void)setupRendererWithView:(MTKView *)view context:(VRORenderContext *)context {
    _scene = std::make_shared<VROScene>();
    _layout = std::make_shared<VROCrossLayout>(_scene);
    
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(1, 1);
    std::shared_ptr<VROBox> box = VROBox::createBox(1, 1, 1);
    
    _rootNode = std::make_shared<VRONode>(*context);
    _rootNode->setGeometry(box);
    _rootNode->setPosition({0, 0, 0});
    
    _scene->addNode(_rootNode);
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, 1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(15);
    
    _rootNode->setLight(light);
    
    _boxNode = std::make_shared<VRONode>(*context);
    _boxNode->setGeometry(box);
    _boxNode->setPosition({0, 0, 5});
    
    _rootNode->addChildNode(_boxNode);
    
    UIImage *moments = [UIImage imageNamed:@"momentslogo"];
    
    std::shared_ptr<VROLayer> center = std::make_shared<VROLayer>(*context);
    center->setContents(moments);
    center->setFrame(VRORectMake(-0.5, -1.5, 2, 1, 1));
    
    _rootNode->addChildNode(center);

    VROView *labelView = [[VROView alloc] initWithFrame:CGRectMake(0, 0, 100, 10) context:context];
    labelView.vroLayer->setFrame(VRORectMake(-1, -2, 2, 2, 0.2));
    
    [labelView setBackgroundColor:[UIColor clearColor]];
    
    UILabel *label = [[UILabel alloc] initWithFrame:labelView.bounds];
    [label setText:@"Moments"];
    [label setBackgroundColor:[UIColor clearColor]];
    [label setTextAlignment:NSTextAlignmentCenter];
    [label setTextColor:[UIColor whiteColor]];
    [label setFont:[UIFont systemFontOfSize:12]];
    
    [labelView addSubview:label];
    [labelView update];
    
    _rootNode->addChildNode(labelView.vroLayer);
    
    /*
     Code for cross-layout.
     
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

     center->addSublayer(labelView.vroLayer);
    
     std::shared_ptr<VROMomentsLayoutDelegate> delegate = std::make_shared<VROMomentsLayoutDelegate>(center, top, bottom, left, right);
     _layout->setDelegate(delegate);
     _layout->layout();
     */
}

- (void)shutdownRendererWithView:(MTKView *)view {
    
}

- (void)prepareNewFrameWithHeadViewMatrix:(matrix_float4x4)headViewMatrix {
    
}

- (void)renderEye:(VROEyeType)eye context:(VRORenderContext *)renderContext {
    angle += .01;
    _boxNode->setRotation({ 0, angle, 0});
    
    _scene->render(*renderContext);
}

- (void)renderViewDidChangeSize:(CGSize)size context:(VRORenderContext *)context {

}

@end

