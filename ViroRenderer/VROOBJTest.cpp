//
//  VROOBJTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROOBJTest.h"
#include "VROTestUtil.h"
#include "VROModelIOUtil.h"

VROOBJTest::VROOBJTest() :
    VRORendererTest(VRORendererTestType::OBJ) {
        
}

VROOBJTest::~VROOBJTest() {
    
}

void VROOBJTest::build(std::shared_ptr<VRORenderer> renderer,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       std::shared_ptr<VRODriver> driver) {
   
    _objAngle = 0;
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 1.0, 1.0 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(50);
    light->setAttenuationEndDistance(75);
    light->setSpotInnerAngle(45);
    light->setSpotOuterAngle(90);
    light->setIntensity(100);
    light->setTemperature(2000);
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    rootNode->setBackgroundCube(VROTestUtil::loadNiagaraBackground());

    std::shared_ptr<VRONode> objNode = loadOBJ(driver);
    rootNode->addChildNode(objNode);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([this](VRONode *const node, float seconds) {
        _objAngle += .0015;
        node->setRotation({ 0, _objAngle, 0});
        
        return true;
    });
    
    objNode->runAction(action);
}

std::shared_ptr<VRONode> VROOBJTest::loadOBJ(std::shared_ptr<VRODriver> driver) {
    std::string url = VROTestUtil::getURLForResource("cupcake", "obj");
    std::string base = url.substr(0, url.find_last_of('/'));
    
    std::shared_ptr<VRONode> objNode = std::make_shared<VRONode>();
    VROOBJLoader::loadOBJFromResource(url, VROResourceType::URL, objNode, driver,
                                      [](std::shared_ptr<VRONode> node, bool success) {
                                          if (!success) {
                                              return;
                                          }
                                          
                                          std::shared_ptr<VROMaterial> material = node->getGeometry()->getMaterials().front();
                                          material->setLightingModel(VROLightingModel::PhysicallyBased);
                                          material->getAmbientOcclusion().setTexture(VROTestUtil::loadTexture("cupcake_vanilla_1001_AO", true));
                                          
                                          node->setPosition({0, -0.35, -1.5});
                                          node->setScale({ 5, 5, 5 });
                                      });
    return objNode;
}
