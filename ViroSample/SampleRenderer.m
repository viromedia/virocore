//
//  GameViewController.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "SampleRenderer.h"

@interface SampleRenderer ()

@property (readwrite, nonatomic) BOOL tapEnabled;

@end

@implementation SampleRenderer {
    VROView *_view;
    std::shared_ptr<VROScene> _scene;
    std::shared_ptr<VROCrossLayout> _layout;
    
    std::shared_ptr<VRONode> _rootNode;
    float angle;
}

- (std::shared_ptr<VROTexture>) cubeTexture {
    std::vector<UIImage *> cubeImages =  {
        [UIImage imageNamed:@"px"],
        [UIImage imageNamed:@"nx"],
        [UIImage imageNamed:@"py"],
        [UIImage imageNamed:@"ny"],
        [UIImage imageNamed:@"pz"],
        [UIImage imageNamed:@"nz"]
    };
    
    return std::make_shared<VROTexture>(cubeImages);
}

- (void)runSphereTest:(VRORenderContext *)context {
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(20);
    
    _rootNode->setLight(light);
    _scene->addNode(_rootNode);
    
    std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(1, 20, 20, false);
    std::shared_ptr<VROMaterial> material = sphere->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Constant);
    
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"surfing" ofType:@"mp4"];
    
    std::shared_ptr<VROVideoTexture> videoTexture = std::make_shared<VROVideoTexture>();
    videoTexture->displayVideo([NSURL fileURLWithPath:filePath], *context);
    
    material->getDiffuse().setContents(videoTexture);
    
    std::shared_ptr<VRONode> sphereNode = std::make_shared<VRONode>(*context);
    sphereNode->setGeometry(sphere);
    sphereNode->setPosition({0, 0, 0});
    
    _rootNode->addChildNode(sphereNode);
    [_view.HUD setReticleEnabled:YES];
}

- (std::shared_ptr<VRONode>) newTorus:(VRORenderContext *)context position:(VROVector3f)position {
    std::shared_ptr<VROTorusKnot> torus = VROTorusKnot::createTorusKnot(3, 8, 0.2, 256, 32);
    std::shared_ptr<VROMaterial> material = torus->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getReflective().setContentsCube([self cubeTexture]);
    
    std::shared_ptr<VRONode> torusNode = std::make_shared<VRONode>(*context);
    torusNode->setGeometry(torus);
    torusNode->setPosition(position);
    torusNode->setPivot({1, 0.5, 0.5});
    
    return torusNode;
}

- (void)runTorusAnimationTest:(VRORenderContext *)context {
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(20);
    
    _rootNode->setLight(light);
    _scene->addNode(_rootNode);
    
    float d = 5;
    
    _rootNode->addChildNode([self newTorus:context position:{ 0,  0, -d}]);
    _rootNode->addChildNode([self newTorus:context position:{ d,  0, -d}]);
    _rootNode->addChildNode([self newTorus:context position:{ 0,  d, -d}]);
    _rootNode->addChildNode([self newTorus:context position:{ d,  d, -d}]);
    _rootNode->addChildNode([self newTorus:context position:{ d, -d, -d}]);
    _rootNode->addChildNode([self newTorus:context position:{-d,  0, -d}]);
    _rootNode->addChildNode([self newTorus:context position:{ 0, -d, -d}]);
    _rootNode->addChildNode([self newTorus:context position:{-d,  d, -d}]);
    _rootNode->addChildNode([self newTorus:context position:{-d, -d, -d}]);
    
    _rootNode->addChildNode([self newTorus:context position:{ 0,  0, d}]);
    _rootNode->addChildNode([self newTorus:context position:{ d,  0, d}]);
    _rootNode->addChildNode([self newTorus:context position:{ 0,  d, d}]);
    _rootNode->addChildNode([self newTorus:context position:{ d,  d, d}]);
    _rootNode->addChildNode([self newTorus:context position:{ d, -d, d}]);
    _rootNode->addChildNode([self newTorus:context position:{-d,  0, d}]);
    _rootNode->addChildNode([self newTorus:context position:{ 0, -d, d}]);
    _rootNode->addChildNode([self newTorus:context position:{-d,  d, d}]);
    _rootNode->addChildNode([self newTorus:context position:{-d, -d, d}]);
    
    [_view.HUD setReticleEnabled:YES];
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([self] (float seconds) {
        angle += .015;
        for (std::shared_ptr<VRONode> &torusNode : _rootNode->getSubnodes()) {
            torusNode->setRotation({ 0, angle, 0});
        }
        
        return true;
    });
    
    _rootNode->runAction(action);
    self.tapEnabled = true;
}

- (void)runBoxAnimationTest:(VRORenderContext *)context {
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(20);
    
    _rootNode->setLight(light);
    _scene->addNode(_rootNode);
    
    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 4, 2);
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getDiffuse().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"boba"]));
    material->getSpecular().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"specular"]));
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>(*context);
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -5});
    
    _rootNode->addChildNode(boxNode);
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(2);
        
        UIImage *image = [UIImage imageNamed:@"bobaraj"];
        material->getDiffuse().setContents(std::make_shared<VROTexture>(image));
        
        boxNode->setPosition({ 0, 0, -3});
        
        VROTransaction::commit();
    });
   
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(3);
        
        material->getDiffuse().setContents({ 0.0, 1.0, 0.0, 1.0});
        boxNode->setPosition({ 0, 0, -6});
        
        VROTransaction::commit();
    });
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([boxNode, self](float seconds) {
        angle += .015;
        boxNode->setRotation({ 0, angle, 0});
        
        return true;
    });
    
    boxNode->runAction(action);
}

- (void)runLayerTest:(VRORenderContext *)context {
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Directional);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
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
    NSURL *videoURL = [NSURL URLWithString:@"https://s3-us-west-2.amazonaws.com/dmoontest/img/Zoe2.mp4"];
    
    std::shared_ptr<VROVideoTexture> videoTexture = std::make_shared<VROVideoTexture>();
    videoTexture->displayVideo(videoURL, *context);
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getDiffuse().setContents(videoTexture);
    material->getSpecular().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"specular"]));
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>(*context);
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 1.5, -5});
    
    _rootNode->addChildNode(boxNode);
    
    /*
     Create the moments icon node.
     */
    std::shared_ptr<VROLayer> center = std::make_shared<VROLayer>(*context);
    center->setContents([UIImage imageNamed:@"momentslogo"]);
    center->setFrame(VRORectMake(-0.5, -1.25, -2, 1, 1));
    
    _rootNode->addChildNode(center);
    
    /*
     Create the label node.
     */
    VROWorldUIView *labelView = [[VROWorldUIView alloc] initWithFrame:CGRectMake(0, 0, 100, 10) context:context];
    labelView.vroLayer->setFrame(VRORectMake(-1, -1.5, -2, 2, 0.2));
    
    [labelView setBackgroundColor:[UIColor clearColor]];
    
    UILabel *label = [[UILabel alloc] initWithFrame:labelView.bounds];
    [label setText:@"Moments"];
    [label setBackgroundColor:[UIColor clearColor]];
    [label setTextAlignment:NSTextAlignmentCenter];
    [label setTextColor:[UIColor whiteColor]];
    [label setFont:[UIFont systemFontOfSize:12]];
    
    [labelView addSubview:label];
    [labelView updateWithContext:context];
    
    _rootNode->addChildNode(labelView.vroLayer);
    
    /*
     Create HUD.
     */
    VROScreenUIView *HUD = _view.HUD;
    
    label = [[UILabel alloc] initWithFrame:CGRectMake(0, 0, HUD.frame.size.width, HUD.frame.size.height)];
    [label setCenter:CGPointMake(HUD.frame.size.width / 2.0, HUD.frame.size.height / 2.0)];
    [label setText:@"++ HUD ++"];
    [label setBackgroundColor:[UIColor clearColor]];
    [label setTextAlignment:NSTextAlignmentCenter];
    [label setTextColor:[UIColor blackColor]];
    [label setFont:[UIFont boldSystemFontOfSize:14]];
    
    [HUD addSubview:label];
    [HUD setNeedsUpdate];
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([boxNode, self](float seconds) {
        angle += .015;
        boxNode->setRotation({ 0, angle, 0});
        
        return true;
    });
    
    boxNode->runAction(action);
}

- (void)runOBJTest:(VRORenderContext *)context {
    NSString *soccerPath = [[NSBundle mainBundle] pathForResource:@"soccerball" ofType:@"obj"];
    NSURL *soccerURL = [NSURL fileURLWithPath:soccerPath];
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Directional);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(15);
    
    _rootNode->setLight(light);
    _scene->addNode(_rootNode);
    
    std::shared_ptr<VRONode> objNode = VROLoader::loadURL(soccerURL, *context)[0];
    objNode->setPosition({0, 0, -20});
    
    _rootNode->addChildNode(objNode);
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(10);
        
        objNode->getGeometry()->getMaterials()[0]->getDiffuse().setContents({1.0, 0.0, 0.0, 1.0});
        VROTransaction::commit();
    });
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([objNode, self](float seconds) {
        angle += .015;
        objNode->setRotation({ 0, angle, 0});
        
        return true;
    });
    
    objNode->runAction(action);
}

- (void)setupRendererWithView:(VROView *)view context:(VRORenderContext *)context {
    _view = view;
    _scene = std::make_shared<VROScene>();
    _layout = std::make_shared<VROCrossLayout>(_scene);

    _scene->setBackground([self cubeTexture]);

    _rootNode = std::make_shared<VRONode>(*context);
    _rootNode->setPosition({0, 0, 0});
    
    //[self runSphereTest:context];
    [self runTorusAnimationTest:context];
    //[self runLayerTest:context];
    //[self runBoxAnimationTest:context];
    //[self runOBJTest:context];
}

- (void)shutdownRendererWithView:(MTKView *)view {
    
}

- (void)prepareNewFrameWithHeadViewMatrix:(matrix_float4x4)headViewMatrix {
    
}

- (void)renderEye:(VROEyeType)eye context:(VRORenderContext *)renderContext {
    _scene->render(*renderContext);
}

- (void)reticleTapped:(VROVector3f)ray {
    if (!self.tapEnabled) {
        return;
    }
    
    std::vector<VROHitTestResult> results = _rootNode->hitTest(ray);
    
    for (VROHitTestResult result : results) {
        std::shared_ptr<VRONode> node = result.getNode();
        std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();
        
        #define ARC4RANDOM_MAX      0x100000000
        float r = ((double)arc4random() / ARC4RANDOM_MAX);
        float g = ((double)arc4random() / ARC4RANDOM_MAX);
        float b = ((double)arc4random() / ARC4RANDOM_MAX);
        
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(1.0);
        VROTransaction::setTimingFunction(VROTimingFunctionType::EaseIn);
        material->getDiffuse().setContents( {r, g, b, 1.0 } );
        VROTransaction::commit();
        
        std::shared_ptr<VROAction> action = VROAction::timedAction([node](float t) {
            float scale = 1.0;
            
            if (t < 0.5) {
                scale = 1.0 - t;
            }
            else {
                scale = t;
            }
            
            node->setScale({scale, scale, scale});
        }, VROTimingFunctionType::Bounce, 1.0);
        node->runAction(action);
    }
}


- (void)renderViewDidChangeSize:(CGSize)size context:(VRORenderContext *)context {

}

@end

