//
//  VROTorusTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROTorusTest.h"
#include "VROTestUtil.h"

VROTorusTest::VROTorusTest() :
    VRORendererTest(VRORendererTestType::Torus) {
        
}

VROTorusTest::~VROTorusTest() {
    
}

void VROTorusTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                         std::shared_ptr<VRODriver> driver) {

    _torusAngle = 0;
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.3, 0.3 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(20);
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    rootNode->setBackgroundCube(VROTestUtil::loadCloudBackground());
    
    float d = 5;
    
    rootNode->addChildNode(newTorus({ 0,  0, -d}));
    rootNode->addChildNode(newTorus({ d,  0, -d}));
    rootNode->addChildNode(newTorus({ 0,  d, -d}));
    rootNode->addChildNode(newTorus({ d,  d, -d}));
    rootNode->addChildNode(newTorus({ d, -d, -d}));
    rootNode->addChildNode(newTorus({-d,  0, -d}));
    rootNode->addChildNode(newTorus({ 0, -d, -d}));
    rootNode->addChildNode(newTorus({-d,  d, -d}));
    rootNode->addChildNode(newTorus({-d, -d, -d}));
    
    rootNode->addChildNode(newTorus({ 0,  0, d}));
    rootNode->addChildNode(newTorus({ d,  0, d}));
    rootNode->addChildNode(newTorus({ 0,  d, d}));
    rootNode->addChildNode(newTorus({ d,  d, d}));
    rootNode->addChildNode(newTorus({ d, -d, d}));
    rootNode->addChildNode(newTorus({-d,  0, d}));
    rootNode->addChildNode(newTorus({ 0, -d, d}));
    rootNode->addChildNode(newTorus({-d,  d, d}));
    rootNode->addChildNode(newTorus({-d, -d, d}));
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([this] (VRONode *const node, float seconds) {
        _torusAngle += .015;
        for (std::shared_ptr<VRONode> &torusNode : node->getChildNodes()) {
            torusNode->setRotation({ 0, _torusAngle, 0});
        }
        return true;
    });
    
    rootNode->runAction(action);
}

std::shared_ptr<VRONode> VROTorusTest::newTorus(VROVector3f position) {
    std::shared_ptr<VROTorusKnot> torus = VROTorusKnot::createTorusKnot(3, 8, 0.2, 256, 32);
    std::shared_ptr<VROMaterial> material = torus->getMaterials()[0];
    material->setLightingModel(VROLightingModel::Lambert);
    material->getReflective().setTexture(VROTestUtil::loadCloudBackground());
    material->getSpecular().setTexture(VROTestUtil::loadSpecularTexture("specular"));
 
    std::shared_ptr<VRONode> torusNode = std::make_shared<VRONode>();
    torusNode->setGeometry(torus);
    torusNode->setPosition(position);
    
    return torusNode;
}

