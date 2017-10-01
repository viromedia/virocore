//
//  VROOBJTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROOBJTest.h"
#include "VROTestUtil.h"

VROOBJTest::VROOBJTest() :
    VRORendererTest(VRORendererTestType::OBJ) {
        
}

VROOBJTest::~VROOBJTest() {
    
}

void VROOBJTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       std::shared_ptr<VRODriver> driver) {
   
    _objAngle = 0;
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::string url = VROTestUtil::getURLForResource("male02", "obj");
    std::string base = url.substr(0, url.find_last_of('/'));
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 0.7, 0.7, 0.7 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(50);
    light->setAttenuationEndDistance(75);
    light->setSpotInnerAngle(45);
    light->setSpotOuterAngle(90);
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    rootNode->setBackgroundCube(VROTestUtil::loadNiagaraBackground());
    
    std::shared_ptr<VRONode> objNode = VROOBJLoader::loadOBJFromURL(url, base, true,
                                                                    [](std::shared_ptr<VRONode> node, bool success) {
                                                                        if (!success) {
                                                                            return;
                                                                        }
                                                                        
                                                                        node->setPosition({0, -10, -20});
                                                                        node->setScale({ 0.1, 0.1, 0.1 });
                                                                    });
    
    
    rootNode->addChildNode(objNode);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([this](VRONode *const node, float seconds) {
        _objAngle += .015;
        node->setRotation({ 0, _objAngle, 0});
        
        return true;
    });
    
    objNode->runAction(action);
}
