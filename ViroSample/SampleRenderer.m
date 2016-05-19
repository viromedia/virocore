//
//  GameViewController.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "SampleRenderer.h"

typedef NS_ENUM(NSInteger, VROSampleScene) {
    VROSampleSceneVideoSphere = 0,
    VROSampleSceneBox,
    VROSampleSceneOBJ,
    VROSampleSceneTorus,
    VROSampleSceneLayer,
    VROSampleSceneNumScenes
};

@interface SampleRenderer ()

@property (readwrite, nonatomic) VRODriver *driver;
@property (readwrite, nonatomic) BOOL tapEnabled;
@property (readwrite, nonatomic) float torusAngle;
@property (readwrite, nonatomic) float boxAngle;
@property (readwrite, nonatomic) float boxVideoAngle;
@property (readwrite, nonatomic) float objAngle;
@property (readwrite, nonatomic) int sceneIndex;
@property (readwrite, nonatomic) std::shared_ptr<VROVideoTexture> videoTexture;

@end

@implementation SampleRenderer

- (std::shared_ptr<VROTexture>) niagaraTexture {
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

- (std::shared_ptr<VROTexture>) cloudTexture {
    std::vector<UIImage *> cubeImages =  {
        [UIImage imageNamed:@"px1.jpg"],
        [UIImage imageNamed:@"nx1.jpg"],
        [UIImage imageNamed:@"py1.jpg"],
        [UIImage imageNamed:@"ny1.jpg"],
        [UIImage imageNamed:@"pz1.jpg"],
        [UIImage imageNamed:@"nz1.jpg"]
    };
    
    return std::make_shared<VROTexture>(cubeImages);
}

- (VROSceneController *)loadVideoSphereScene {
    VROSceneController *sceneController = [[VROSceneController alloc] initWithView:self.view];
    std::shared_ptr<VROScene> scene = sceneController.scene;
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(20);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->setLight(light);
    
    scene->addNode(rootNode);
    
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"surfing" ofType:@"mp4"];
    
    self.videoTexture = std::make_shared<VROVideoTexture>();
    self.videoTexture->loadVideo([NSURL fileURLWithPath:filePath], [self.view frameSynchronizer], *self.driver);
    self.videoTexture->play();
    
    scene->setBackgroundSphere(self.videoTexture);
    
    [self.view.HUD setReticleEnabled:YES];
    
    return sceneController;
}

- (std::shared_ptr<VRONode>) newTorusWithPosition:(VROVector3f)position {
    std::shared_ptr<VROTorusKnot> torus = VROTorusKnot::createTorusKnot(3, 8, 0.2, 256, 32);
    std::shared_ptr<VROMaterial> material = torus->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getReflective().setContentsCube([self cloudTexture]);

    
    std::shared_ptr<VRONode> torusNode = std::make_shared<VRONode>();
    torusNode->setGeometry(torus);
    torusNode->setPosition(position);
    torusNode->setPivot({1, 0.5, 0.5});
    
    return torusNode;
}

- (VROSceneController *)loadTorusScene {
    VROSceneController *sceneController = [[VROSceneController alloc] initWithView:self.view];
    std::shared_ptr<VROScene> scene = sceneController.scene;
    scene->setBackgroundCube([self cloudTexture]);
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(20);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->setLight(light);
    
    scene->addNode(rootNode);
    
    float d = 5;
    
    rootNode->addChildNode([self newTorusWithPosition:{ 0,  0, -d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ d,  0, -d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ 0,  d, -d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ d,  d, -d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ d, -d, -d}]);
    rootNode->addChildNode([self newTorusWithPosition:{-d,  0, -d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ 0, -d, -d}]);
    rootNode->addChildNode([self newTorusWithPosition:{-d,  d, -d}]);
    rootNode->addChildNode([self newTorusWithPosition:{-d, -d, -d}]);
    
    rootNode->addChildNode([self newTorusWithPosition:{ 0,  0, d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ d,  0, d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ 0,  d, d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ d,  d, d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ d, -d, d}]);
    rootNode->addChildNode([self newTorusWithPosition:{-d,  0, d}]);
    rootNode->addChildNode([self newTorusWithPosition:{ 0, -d, d}]);
    rootNode->addChildNode([self newTorusWithPosition:{-d,  d, d}]);
    rootNode->addChildNode([self newTorusWithPosition:{-d, -d, d}]);
    
    [self.view.HUD setReticleEnabled:YES];
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([self] (VRONode *const node, float seconds) {
        self.torusAngle += .015;
        for (std::shared_ptr<VRONode> &torusNode : node->getSubnodes()) {
            torusNode->setRotation({ 0, self.torusAngle, 0});
        }
        
        return true;
    });
    
    rootNode->runAction(action);
    self.tapEnabled = true;
    
    [sceneController setHoverEnabled:true boundsOnly:true];
    return sceneController;
}

- (void)hoverOn:(std::shared_ptr<VRONode>)node {
    std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();
    
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(0.2);
    material->getDiffuse().setContents( {1.0, 1.0, 1.0, 1.0 } );
    material->getReflective().setContentsCube([self cloudTexture]);
    VROTransaction::commit();
}

- (void)hoverOff:(std::shared_ptr<VRONode>)node {
    std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();
    
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(0.2);
    material->getDiffuse().setContents( {1.0, 1.0, 1.0, 1.0 } );
    material->getReflective().clear();
    VROTransaction::commit();
}

- (VROSceneController *)loadBoxScene {
    VROSceneController *sceneController = [[VROSceneController alloc] initWithView:self.view];

    std::shared_ptr<VROScene> scene = sceneController.scene;
    scene->setBackgroundCube([self niagaraTexture]);
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 1.0, 1.0 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(8);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(80);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->setLight(light);
    
    scene->addNode(rootNode);
    
    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 4, 2);
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Phong);
    material->getDiffuse().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"boba"]));
    material->getSpecular().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"specular"]));
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -5});
    
    rootNode->addChildNode(boxNode);
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(2);
        
        UIImage *image = [UIImage imageNamed:@"boba"];
        material->getDiffuse().setContents(std::make_shared<VROTexture>(image));
        
        boxNode->setPosition({ 0, 0, -3});
        
        VROTransaction::commit();
    });
   
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(3);
        
        //material->getDiffuse().setContents({ 0.0, 1.0, 0.0, 1.0});
        boxNode->setPosition({ 0, 0, -6});
        
        VROTransaction::commit();
    });

    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([self](VRONode *const node, float seconds) {
        self.boxAngle += .015;
        node->setRotation({ 0, self.boxAngle, 0});
        
        return true;
    });
    
    boxNode->runAction(action);
    return sceneController;
}

- (VROSceneController *)loadLayerScene {
    VROSceneController *sceneController = [[VROSceneController alloc] initWithView:self.view];

    std::shared_ptr<VROScene> scene = sceneController.scene;
    scene->setBackgroundCube([self cloudTexture]);
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Omni);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(15);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->setLight(light);
    
    scene->addNode(rootNode);
    
    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(1, 1, 1);
    NSURL *videoURL = [NSURL URLWithString:@"https://s3-us-west-2.amazonaws.com/dmoontest/img/Zoe2.mp4"];
    
    std::shared_ptr<VROVideoTexture> videoTexture = std::make_shared<VROVideoTexture>();
    videoTexture->loadVideo(videoURL, [self.view frameSynchronizer], *self.driver);
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getDiffuse().setContents(videoTexture);
    material->getSpecular().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"specular"]));
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -5});
    
    rootNode->addChildNode(boxNode);
    
    /*
     Create the moments icon node.
     */
    std::shared_ptr<VROLayer> center = std::make_shared<VROLayer>();
    center->setContents([UIImage imageNamed:@"momentslogo"]);
    center->setFrame(VRORectMake(3.0, -1.25, 2, 1, 1));
    center->addConstraint(std::make_shared<VROBillboardConstraint>());
    
    rootNode->addChildNode(center);
    
    /*
     Create the label node.
     */
    VROWorldUIView *labelView = [[VROWorldUIView alloc] initWithFrame:CGRectMake(0, 0, 100, 10)];
    labelView.vroLayer->setFrame(VRORectMake(0, 0, -5, 2, 0.2));
    
    [labelView setBackgroundColor:[UIColor clearColor]];
    
    UILabel *label = [[UILabel alloc] initWithFrame:labelView.bounds];
    [label setText:@"Moments"];
    [label setBackgroundColor:[UIColor clearColor]];
    [label setTextAlignment:NSTextAlignmentCenter];
    [label setTextColor:[UIColor whiteColor]];
    [label setFont:[UIFont systemFontOfSize:12]];
    
    [labelView addSubview:label];
    [labelView updateWithContext:self.driver];
    
    rootNode->addChildNode(labelView.vroLayer);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([self](VRONode *const node, float seconds) {
        self.boxVideoAngle += .015;
        node->setRotation({ 0, self.boxVideoAngle, 0});
        
        return true;
    });
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(3 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(5.0);
        
        //[self.view setPosition:{0, 0, -4}];
        
        VROTransaction::commit();
    });
    
    boxNode->runAction(action);
    [self.view.HUD setReticleEnabled:YES];
    self.tapEnabled = YES;

    return sceneController;
}

- (VROSceneController *)loadOBJScene {
    VROSceneController *sceneController = [[VROSceneController alloc] initWithView:self.view];
    std::shared_ptr<VROScene> scene = sceneController.scene;
    scene->setBackgroundCube([self niagaraTexture]);
    
    NSString *soccerPath = [[NSBundle mainBundle] pathForResource:@"shanghai_tower" ofType:@"obj"];
    NSURL *soccerURL = [NSURL fileURLWithPath:soccerPath];
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(15);
    light->setAttenuationEndDistance(25);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(15);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->setLight(light);
    
    scene->addNode(rootNode);
    
    std::shared_ptr<VRONode> objNode = VROLoader::loadURL(soccerURL)[0];
    objNode->getGeometry()->getMaterials()[0]->getDiffuse().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"shanghai_tower_diffuse.jpg"]));
    objNode->getGeometry()->getMaterials()[0]->getSpecular().setContents(std::make_shared<VROTexture>([UIImage imageNamed:@"shanghai_tower_specular.jpg"]));
    objNode->setPosition({0, -10, -20});
    
    rootNode->addChildNode(objNode);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([self](VRONode *const node, float seconds) {
        self.objAngle += .015;
        node->setRotation({ 0, self.objAngle, 0});
        
        return true;
    });
    
    objNode->runAction(action);
    return sceneController;
}

- (VROSceneController *)loadSceneWithIndex:(int)index {
    int modulo = index % VROSampleSceneNumScenes;
    
    switch (modulo) {
        case VROSampleSceneTorus:
            return [self loadTorusScene];
        case VROSampleSceneVideoSphere:
            return [self loadVideoSphereScene];
        case VROSampleSceneLayer:
            return [self loadLayerScene];
        case VROSampleSceneOBJ:
            return [self loadOBJScene];
        case VROSampleSceneBox:
            return [self loadBoxScene];
        default:
            break;
    }
    
    return [self loadTorusScene];
}

- (void)setupRendererWithDriverContext:(VRODriver *)driver {
    self.driver = driver;
    self.view.sceneController = [self loadSceneWithIndex:self.sceneIndex];
}

- (IBAction)nextScene:(id)sender {
    ++self.sceneIndex;
    
    VROSceneController *sceneController = [self loadSceneWithIndex:self.sceneIndex];
    [self.view setSceneController:sceneController animated:YES];
}

- (void)shutdownRenderer {
    
}

- (void)willRenderEye:(VROEyeType)eye context:(const VRORenderContext *)renderContext {

}

- (void)didRenderEye:(VROEyeType)eye context:(const VRORenderContext *)renderContext {

}

- (void)reticleTapped:(VROVector3f)ray context:(const VRORenderContext *)renderContext {
    if (self.videoTexture) {
        if (self.videoTexture->isPaused()) {
            self.videoTexture->play();
        }
        else {
            self.videoTexture->pause();
        }
        
        return;
    }
    
    if (!self.tapEnabled) {
        return;
    }
    
    std::vector<VROHitTestResult> results = self.view.sceneController.scene->hitTest(ray, *renderContext);
    
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
        
        std::shared_ptr<VROAction> action = VROAction::timedAction([](VRONode *const node, float t) {
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

