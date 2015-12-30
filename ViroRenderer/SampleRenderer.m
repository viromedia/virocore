//
//  GameViewController.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "SampleRenderer.h"

@implementation SampleRenderer {
    std::shared_ptr<VROScene> _scene;
    std::shared_ptr<VROCrossLayout> _layout;
    
    std::shared_ptr<VRONode> _rootNode;
    std::shared_ptr<VRONode> _boxNode;
    float angle;
}

- (void)setupRendererWithView:(VROView *)view context:(VRORenderContext *)context {
    _scene = std::make_shared<VROScene>();
    _layout = std::make_shared<VROCrossLayout>(_scene);
    
    /*
     Create the root node.
     */
    _rootNode = std::make_shared<VRONode>(*context);
    _rootNode->setPosition({0, 0, 0});
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, 1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(15);
    
    _rootNode->setLight(light);
    _scene->addNode(_rootNode);

    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(1, 1, 1);
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getDiffuse().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"boba"]));
    material->getSpecular().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"specular"]));
    material->setShininess(2.0);

    _boxNode = std::make_shared<VRONode>(*context);
    _boxNode->setGeometry(box);
    _boxNode->setPosition({0, 0, 5});
    
    _rootNode->addChildNode(_boxNode);
    
    /*
     Create the moments icon node.
     */
    std::shared_ptr<VROLayer> center = std::make_shared<VROLayer>(*context);
    center->setContents([UIImage imageNamed:@"momentslogo"]);
    center->setFrame(VRORectMake(-0.5, -1.5, 2, 1, 1));
    
    _rootNode->addChildNode(center);

    /*
     Create the label node.
     */
    VROUIView *labelView = [[VROUIView alloc] initWithFrame:CGRectMake(0, 0, 100, 10) context:context];
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
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(1);
        
        UIImage *image = [UIImage imageNamed:@"bobaraj"];
        material->getDiffuse().setContents(std::make_shared<VROTexture>(image));
    });
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

