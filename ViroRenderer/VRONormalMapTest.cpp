//
//  VRONormalMapTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VRONormalMapTest.h"
#include "VROTestUtil.h"

VRONormalMapTest::VRONormalMapTest() :
    VRORendererTest(VRORendererTestType::NormalMap) {
        
}

VRONormalMapTest::~VRONormalMapTest() {
    
}

void VRONormalMapTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                             std::shared_ptr<VRODriver> driver) {
    
    _objAngle = 0;
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::string url = VROTestUtil::getURLForResource("earth", "obj");
    std::string base = url.substr(0, url.find_last_of('/'));
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Directional);
    light->setColor({ 0.9, 0.9, 0.9 });
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
                                                                        
                                                                        node->setPosition({0, 0, -5.5});
                                                                        node->setScale({ 0.01, 0.01, 0.01 });
                                                                    });
    
    rootNode->addChildNode(objNode);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([this](VRONode *const node, float seconds) {
        _objAngle += .010;
        node->setRotation({ 0, _objAngle, 0});
        
        return true;
    });
    
    objNode->runAction(action);
}
