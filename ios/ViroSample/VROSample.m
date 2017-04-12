//
//  VROSample.m
//  ViroRenderer
//
//  Created by Raj Advani on 10/13/15.
//  Copyright © 2015 Raj Advani. All rights reserved.
//

#import "VROSample.h"

typedef NS_ENUM(NSInteger, VROSampleScene) {
    VROSampleSceneOBJ = 0,
    VROSampleSceneTorus,
    VROSampleSceneCamera,
    VROSampleSceneBox,
    VROSampleSceneText,
    VROSampleSceneVideoSphere,
    VROSampleSceneNormalMap,
    VROSampleSceneNumScenes,
    VROSampleStereoscopic
};

@interface VROSample ()

@property (readwrite, nonatomic) std::shared_ptr<VRODriver> driver;
@property (readwrite, nonatomic) BOOL tapEnabled;
@property (readwrite, nonatomic) float torusAngle;
@property (readwrite, nonatomic) float boxAngle;
@property (readwrite, nonatomic) float boxVideoAngle;
@property (readwrite, nonatomic) float objAngle;
@property (readwrite, nonatomic) int sceneIndex;
@property (readwrite, nonatomic) std::shared_ptr<VROVideoTexture> videoTexture;

@end

@implementation VROSample

- (std::shared_ptr<VROSceneController>)loadSceneWithIndex:(int)index {
    int modulo = index % VROSampleSceneNumScenes;
    
    
    return [self loadStereoBackground];
}

- (std::shared_ptr<VROTexture>) niagaraTexture {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
    
    std::vector<std::shared_ptr<VROImage>> cubeImages =  {
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"px"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"nx"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"py"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"ny"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"pz"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"nz"], format)
    };
    
    return std::make_shared<VROTexture>(format, cubeImages);
}

- (std::shared_ptr<VROTexture>) cloudTexture {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
    
    std::vector<std::shared_ptr<VROImage>> cubeImages =  {
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"px1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"nx1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"py1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"ny1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"pz1.jpg"], format),
        std::make_shared<VROImageiOS>([UIImage imageNamed:@"nz1.jpg"], format)
    };
    
    return std::make_shared<VROTexture>(format, cubeImages);
}

- (std::shared_ptr<VROTexture>) westlakeTexture {
    return std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8, VROMipmapMode::None,
                                        std::make_shared<VROImageiOS>([UIImage imageNamed:@"360_westlake.jpg"],
                                                                      VROTextureInternalFormat::RGBA8));
}

- (std::shared_ptr<VROSceneController>)loadVideoSphereScene {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    
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
    rootNode->addLight(light);
    
    scene->addNode(rootNode);
    
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"surfing" ofType:@"mp4"];
    NSURL *fileURL = [NSURL fileURLWithPath:filePath];
    std::string url = std::string([[fileURL description] UTF8String]);
    
    self.videoTexture = std::make_shared<VROVideoTextureiOS>();
    self.videoTexture->loadVideo(url, [self.view frameSynchronizer], self.driver);
    self.videoTexture->play();
    
    scene->setBackgroundSphere(self.videoTexture);
    return sceneController;
}

- (std::shared_ptr<VRONode>) newTorusWithPosition:(VROVector3f)position {
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;

    std::shared_ptr<VROTorusKnot> torus = VROTorusKnot::createTorusKnot(3, 8, 0.2, 256, 32);
    std::shared_ptr<VROMaterial> material = torus->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Lambert);
    material->getReflective().setTexture([self cloudTexture]);
    material->getSpecular().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                    std::make_shared<VROImageiOS>([UIImage imageNamed:@"specular"], format)));

    
    std::shared_ptr<VRONode> torusNode = std::make_shared<VRONode>();
    torusNode->setGeometry(torus);
    torusNode->setPosition(position);
    torusNode->setPivot({1, 0.5, 0.5});
    
    return torusNode;
}

- (std::shared_ptr<VROSceneController>)loadTorusScene {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube([self cloudTexture]);
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.3, 0.3 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(20);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    
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
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([self] (VRONode *const node, float seconds) {
        self.torusAngle += .015;
        for (std::shared_ptr<VRONode> &torusNode : node->getSubnodes()) {
            torusNode->setRotation({ 0, self.torusAngle, 0});
        }
        
        return true;
    });
    
    rootNode->runAction(action);
    self.tapEnabled = true;
    return sceneController;
}

- (void)hoverOn:(std::shared_ptr<VRONode>)node {
    std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();
    
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(0.2);
    material->getDiffuse().setColor( {1.0, 1.0, 1.0, 1.0 } );
    material->getReflective().setTexture([self cloudTexture]);
    VROTransaction::commit();
}

- (void)hoverOff:(std::shared_ptr<VRONode>)node {
    std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();
    
    VROTransaction::begin();
    VROTransaction::setAnimationDuration(0.2);
    material->getDiffuse().setColor( {1.0, 1.0, 1.0, 1.0 } );
    material->getReflective().clear();
    VROTransaction::commit();
}

- (std::shared_ptr<VROSceneController>)loadBoxScene {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundSphere([self westlakeTexture]);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    
    std::shared_ptr<VROLight> spotRed = std::make_shared<VROLight>(VROLightType::Spot);
    spotRed->setColor({ 1.0, 0.0, 0.0 });
    spotRed->setPosition( { -5, 0, 0 });
    spotRed->setDirection( { 1.0, 0, -1.0 });
    spotRed->setAttenuationStartDistance(20);
    spotRed->setAttenuationEndDistance(30);
    spotRed->setSpotInnerAngle(2.5);
    spotRed->setSpotOuterAngle(5.0);
    
    std::shared_ptr<VROLight> spotBlue = std::make_shared<VROLight>(VROLightType::Spot);
    spotBlue->setColor({ 0.0, 0.0, 1.0 });
    spotBlue->setPosition( { 5, 0, 0 });
    spotBlue->setDirection( { -1.0, 0, -1.0 });
    spotBlue->setAttenuationStartDistance(20);
    spotBlue->setAttenuationEndDistance(30);
    spotBlue->setSpotInnerAngle(2.5);
    spotBlue->setSpotOuterAngle(5.0);
    
    rootNode->addLight(ambient);
    rootNode->addLight(spotRed);
    rootNode->addLight(spotBlue);

    scene->addNode(rootNode);
    
    VROTextureInternalFormat format = VROTextureInternalFormat::RGBA8;
    
    /*
     Create the box node.
     */
    std::shared_ptr<VROBox> box = VROBox::createBox(2, 4, 2);
    box->setName("Box 1");
    
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Blinn);
    material->getDiffuse().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                   std::make_shared<VROImageiOS>([UIImage imageNamed:@"boba"], format)));
    material->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    material->getSpecular().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                    std::make_shared<VROImageiOS>([UIImage imageNamed:@"specular"], format)));
    
    /*
     This geometry modifier pushes the first box to the right slightly, by changing _geometry.position.
     */
    std::vector<std::string> modifierCode =  { "uniform float testA;",
                                               "uniform float testB;",
                                               "_geometry.position.x = _geometry.position.x + testA;"
                                             };
    std::shared_ptr<VROShaderModifier> modifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Geometry,
                                                                                      modifierCode);
    
    modifier->setUniformBinder("testA", [](VROUniform *uniform, GLuint location) {
        uniform->setFloat(1.0);
    });
    material->addShaderModifier(modifier);
    
    /*
     This surface modifier doubles the V tex-coord, making boba.png cover only the top half of the box.
     It then shades the entire surface slightly blue.
     */
    std::vector<std::string> surfaceModifierCode =  {
        "uniform lowp vec3 surface_color;",
        "_surface.diffuse_texcoord.y = _surface.diffuse_texcoord.y * 2.0;"
        "_surface.diffuse_color = vec4(surface_color.xyz, 1.0);"
    };
    std::shared_ptr<VROShaderModifier> surfaceModifier = std::make_shared<VROShaderModifier>(VROShaderEntryPoint::Surface,
                                                                                             surfaceModifierCode);
    
    surfaceModifier->setUniformBinder("surface_color", [](VROUniform *uniform, GLuint location) {
        uniform->setVec3({0.6, 0.6, 1.0});
    });
    material->addShaderModifier(surfaceModifier);
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({0, 0, -5});

    rootNode->addChildNode(boxNode);
    boxNode->addConstraint(std::make_shared<VROBillboardConstraint>(VROBillboardAxis::All));
    
    /*
     Create a second box node behind the first.
     */
    std::shared_ptr<VROBox> box2 = VROBox::createBox(2, 4, 2);
    box2->setName("Box 2");
    
    std::shared_ptr<VROMaterial> material2 = box2->getMaterials()[0];
    material2->setLightingModel(VROLightingModel::Phong);
    material2->getDiffuse().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                    std::make_shared<VROImageiOS>([UIImage imageNamed:@"boba"], format)));
    material2->getSpecular().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                     std::make_shared<VROImageiOS>([UIImage imageNamed:@"specular"], format)));
    
    std::shared_ptr<VRONode> boxNode2 = std::make_shared<VRONode>();
    boxNode2->setGeometry(box2);
    boxNode2->setPosition({0, 0, -9});
    
    rootNode->addChildNode(boxNode2);
    
    /*
     Create a third box node behind the second.
     */
    std::shared_ptr<VROBox> box3 = VROBox::createBox(2, 4, 2);
    box3->setName("Box 3");
    
    std::shared_ptr<VROMaterial> material3 = box3->getMaterials()[0];
    material3->setLightingModel(VROLightingModel::Lambert);
    material3->getDiffuse().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                    std::make_shared<VROImageiOS>([UIImage imageNamed:@"boba"], format)));
    material3->getSpecular().setTexture(std::make_shared<VROTexture>(format, VROMipmapMode::None,
                                                                     std::make_shared<VROImageiOS>([UIImage imageNamed:@"specular"], format)));
    
    std::shared_ptr<VRONode> boxNode3 = std::make_shared<VRONode>();
    boxNode3->setGeometry(box3);
    boxNode3->setPosition({0, 0, -13});
    
    rootNode->addChildNode(boxNode3);
    
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    camera->setRotationType(VROCameraRotationType::Orbit);
    camera->setOrbitFocalPoint(boxNode->getPosition());
    camera->setPosition({0, -5, 0});
    
    std::shared_ptr<VRONode> cameraNode = std::make_shared<VRONode>();
    cameraNode->setCamera(camera);
    rootNode->addChildNode(cameraNode);
    
    [self.view setPointOfView:cameraNode];

    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(6);
        
        spotRed->setPosition({5, 0, 0});
        spotRed->setDirection({-1, 0, -1});
        
        spotBlue->setPosition({-5, 0, 0});
        spotBlue->setDirection({1, 0, -1});
        
        cameraNode->setPosition({0, 1, 0});
        
        VROTransaction::commit();
    });

    return sceneController;
}

- (std::shared_ptr<VROSceneController>)loadCameraScene {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube([self cloudTexture]);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    
    scene->addNode(rootNode);
    
    /*
     Create camera texture.
     */
    int width = 10;
    int height = 10;

    std::shared_ptr<VROCameraTextureiOS> texture = std::make_shared<VROCameraTextureiOS>(VROTextureType::Texture2D);
    texture->initCamera(VROCameraPosition::Back, self.driver);
    texture->play();
    
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(10 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        texture->pause();
    });
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(20 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        texture->play();
    });
    
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height);
    surface->getMaterials().front()->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    surface->getMaterials().front()->getDiffuse().setTexture(texture);
    
    std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
    surfaceNode->setGeometry(surface);
    surfaceNode->setPosition({0, 0, -5.01});
    
    rootNode->addChildNode(surfaceNode);
    
    self.tapEnabled = YES;
    
    return sceneController;
}

- (std::shared_ptr<VROSceneController>)loadTextScene {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube([self cloudTexture]);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    
    scene->addNode(rootNode);
    
    /*
     Create background for text.
     */
    int width = 10;
    int height = 10;
    
    NSString *filePath = [[NSBundle mainBundle] pathForResource:@"card_main" ofType:@"ktx"];
    NSURL *fileURL = [NSURL fileURLWithPath:filePath];
    NSData *fileData = [NSData dataWithContentsOfURL:fileURL];
    
    VROTextureFormat format;
    int texWidth;
    int texHeight;
    std::vector<uint32_t> mipSizes;
    std::shared_ptr<VROData> texData = VROTextureUtil::readKTXHeader((uint8_t *)[fileData bytes], (uint32_t)[fileData length],
                                                                     &format, &texWidth, &texHeight, &mipSizes);
    std::vector<std::shared_ptr<VROData>> dataVec = { texData };
    
    std::shared_ptr<VROTexture> texture = std::make_shared<VROTexture>(VROTextureType::Texture2D, format,
                                                                       VROTextureInternalFormat::RGBA8,
                                                                       VROMipmapMode::Pregenerated,
                                                                       dataVec, texWidth, texHeight, mipSizes);
    texture->prewarm(self.driver);
    
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(width, height, 0.5, 0.5, 1, 1);
    surface->getMaterials().front()->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    surface->getMaterials().front()->getDiffuse().setTexture(texture);
    
    std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
    surfaceNode->setGeometry(surface);
    surfaceNode->setPosition({0, 0, -10.01});
    
    rootNode->addChildNode(surfaceNode);
    
    /*
     Create text.
     */
    VROLineBreakMode linebreakMode = VROLineBreakMode::Justify;
    VROTextClipMode clipMode = VROTextClipMode::ClipToBounds;
    
    std::shared_ptr<VROTypeface> typeface = self.driver->newTypeface("SF", 26);
    //std::string string = "Hello Freetype, this is a test of wrapping a long piece of text, longer than all the previous pieces of text.";
    
    std::string string = "In older times when wishing still helped one, there lived a king whose daughters were all beautiful; and the youngest was so beautiful that the sun itself, which has seen so much, was astonished whenever it shone in her face.\n\nClose by the king's castle lay a great dark forest, and under an old lime-tree in the forest was a well, and when the day was very warm, the king's child went out to the forest and sat down by the fountain; and when she was bored she took a golden ball, and threw it up on high and caught it; and this ball was her favorite plaything.";
    
    VROVector3f size = VROText::getTextSize(string, typeface, width, height, linebreakMode, clipMode, 0);
    NSLog(@"Estimated size %f, %f", size.x, size.y);
    
    std::shared_ptr<VROText> text = VROText::createText(string, typeface, {1.0, 0.0, 0.0, 1.0}, width, height,
                                                        VROTextHorizontalAlignment::Left, VROTextVerticalAlignment::Top,
                                                        linebreakMode, clipMode);
    
    text->setName("Text");
    NSLog(@"Realized size %f, %f", text->getWidth(), text->getHeight());
    
    std::shared_ptr<VRONode> textNode = std::make_shared<VRONode>();
    textNode->setGeometry(text);
    textNode->setPosition({0, 0, -10});
    
    rootNode->addChildNode(textNode);
    self.tapEnabled = YES;

    return sceneController;
}

- (std::shared_ptr<VROSceneController>)loadNormalMapScene {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube([self niagaraTexture]);
    
    NSString *objPath = [[NSBundle mainBundle] pathForResource:@"earth" ofType:@"obj"];
    NSURL *objURL = [NSURL fileURLWithPath:objPath];
    std::string url = std::string([[objURL description] UTF8String]);
    
    NSString *basePath = [objPath stringByDeletingLastPathComponent];
    NSURL *baseURL = [NSURL fileURLWithPath:basePath];
    std::string base = std::string([[baseURL description] UTF8String]);
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Directional);
    light->setColor({ 0.9, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(50);
    light->setAttenuationEndDistance(75);
    light->setSpotInnerAngle(45);
    light->setSpotOuterAngle(90);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    
    scene->addNode(rootNode);
    
    std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromURL(url, base, true,
        [](std::shared_ptr<VRONode> node, bool success) {
            if (!success) {
                return;
            }
            
            node->setPosition({0, 0, -1.5});
            node->setScale({ 0.01, 0.01, 0.01 });
    });
    
    
    rootNode->addChildNode(objNode);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([self](VRONode *const node, float seconds) {
        self.objAngle += .010;
        node->setRotation({ 0, self.objAngle, 0});
        
        return true;
    });
    
    objNode->runAction(action);
    return sceneController;
}

- (std::shared_ptr<VROSceneController>)loadStereoCard {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube([self niagaraTexture]);
    std::shared_ptr<VROSurface> surface = VROSurface::createSurface(20, 20);

    // Debug toggle between stereo image and stereo video cards
    bool showImage = true;
  
    if (showImage){
        std::shared_ptr<VROTexture> imgTexture
        = std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8,
                                       VROMipmapMode::None, // Don't mipmap 360 images, wastes memory
                                       std::make_shared<VROImageiOS>([UIImage imageNamed:@"stereo1.jpg"], VROTextureInternalFormat::RGBA8),
                                       VROStereoMode::LeftRight);
        surface->getMaterials().front()->getDiffuse().setTexture(imgTexture);
    } else {
        // show video
        NSString *objPath = [[NSBundle mainBundle] pathForResource:@"stereoVid" ofType:@"mp4"];
        NSURL *objURL = [NSURL fileURLWithPath:objPath];
        std::string url = std::string([[objURL description] UTF8String]);
        
        self.videoTexture = std::make_shared<VROVideoTextureiOS>(VROStereoMode::LeftRight);
        self.videoTexture->loadVideo(url, [self.view frameSynchronizer], self.driver);
        self.videoTexture->play();
        surface->getMaterials().front()->getDiffuse().setTexture(self.videoTexture);
    }
    
    std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
    surfaceNode->setGeometry(surface);
    surfaceNode->setPosition({0, 0, -35.01});
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 0.7, 0.7, 0.7 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(50);
    light->setAttenuationEndDistance(75);
    light->setSpotInnerAngle(45);
    light->setSpotOuterAngle(90);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    rootNode->addChildNode(surfaceNode);
    scene->addNode(rootNode);
    return sceneController;
}

- (std::shared_ptr<VROSceneController>)loadStereoBackground {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube([self niagaraTexture]);
    // Debug toggle between stereo image and stereo video background
    bool showImage = false;

    if (showImage){
        std::shared_ptr<VROTexture> imgTexture
        = std::make_shared<VROTexture>(VROTextureInternalFormat::RGBA8,
                                       VROMipmapMode::None, // Don't mipmap 360 images, wastes memory
                                       std::make_shared<VROImageiOS>([UIImage imageNamed:@"stereo3601.jpg"], VROTextureInternalFormat::RGBA8),
                                       VROStereoMode::BottomTop);
        scene->setBackgroundSphere(imgTexture);
    } else {
        NSString *objPath = [[NSBundle mainBundle] pathForResource:@"stereoVid360" ofType:@"mp4"];
        NSURL *objURL = [NSURL fileURLWithPath:objPath];
        std::string url = std::string([[objURL description] UTF8String]);
        
        self.videoTexture = std::make_shared<VROVideoTextureiOS>(VROStereoMode::BottomTop);
        self.videoTexture->loadVideo(url, [self.view frameSynchronizer], self.driver);
        self.videoTexture->play();
        scene->setBackgroundSphere(self.videoTexture);

    }
    
    return sceneController;
}

- (std::shared_ptr<VROSceneController>)loadOBJScene {
    std::shared_ptr<VROSceneController> sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = sceneController->getScene();
    scene->setBackgroundCube([self niagaraTexture]);
    
    NSString *objPath = [[NSBundle mainBundle] pathForResource:@"male02" ofType:@"obj"];
    NSURL *objURL = [NSURL fileURLWithPath:objPath];
    std::string url = std::string([[objURL description] UTF8String]);
    
    NSString *basePath = [objPath stringByDeletingLastPathComponent];
    NSURL *baseURL = [NSURL fileURLWithPath:basePath];
    std::string base = std::string([[baseURL description] UTF8String]);
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 0.7, 0.7, 0.7 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(50);
    light->setAttenuationEndDistance(75);
    light->setSpotInnerAngle(45);
    light->setSpotOuterAngle(90);
    
    std::shared_ptr<VRONode> rootNode = std::make_shared<VRONode>();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    
    scene->addNode(rootNode);
    
    std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromURL(url, base, true,
        [](std::shared_ptr<VRONode> node, bool success) {
            if (!success) {
                return;
            }
            
            node->setPosition({0, -100, -10});
            node->setScale({ 0.1, 0.1, 0.1 });
    });
    
    
    rootNode->addChildNode(objNode);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([self](VRONode *const node, float seconds) {
        self.objAngle += .015;
        node->setRotation({ 0, self.objAngle, 0});
        
        return true;
    });
    
    objNode->runAction(action);
    return sceneController;
}

- (void)setupRendererWithDriver:(std::shared_ptr<VRODriver>)driver {
    self.sceneIndex = VROSampleSceneNormalMap;
    self.driver = driver;
    self.view.sceneController = [self loadSceneWithIndex:self.sceneIndex];
    
    [self nextSceneAfterDelay];
}

- (void)nextSceneAfterDelay {
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(5 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        [self nextScene:nullptr];
        [self nextSceneAfterDelay];
    });
}

- (IBAction)nextScene:(id)sender {
    ++self.sceneIndex;
    
    std::shared_ptr<VROSceneController> sceneController = [self loadSceneWithIndex:self.sceneIndex];
    [self.view setSceneController:sceneController];
}

- (void)shutdownRenderer {
    
}

- (void)willRenderEye:(VROEyeType)eye context:(const VRORenderContext *)renderContext {

}

- (void)didRenderEye:(VROEyeType)eye context:(const VRORenderContext *)renderContext {

}

- (void)reticleTapped:(VROVector3f)ray context:(const VRORenderContext *)renderContext {
    if (true) {
        [self nextScene:nullptr];
        return;
    }
    
    if (self.videoTexture) {
        if (self.videoTexture->isPaused()) {
            self.videoTexture->play();
        }
        else {
            self.videoTexture->pause();
        }
        
        return;
    }
}

- (void)renderViewDidChangeSize:(CGSize)size context:(VRORenderContext *)context {

}

- (void)userDidRequestExitVR {
    
}

@end

