//
//  VROPhysicsTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPhysicsTest.h"
#include "VROTestUtil.h"

VROPhysicsTest::VROPhysicsTest() :
    VRORendererTest(VRORendererTestType::Physics) {
        
}

VROPhysicsTest::~VROPhysicsTest() {
    
}

void VROPhysicsTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                   std::shared_ptr<VRODriver> driver) {

    _eventDelegate = std::make_shared<VROPhysicsEventDelegate>(this);
    
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
    std::shared_ptr<VROBox> groundBox = VROBox::createBox(40, 1, 40);
    groundBox->setName("Box 2");
    std::shared_ptr<VROMaterial> materialGround = groundBox->getMaterials()[0];
    materialGround->getDiffuse().setColor({1.0, 1.0, 1.0, 1.0});
    materialGround->setLightingModel(VROLightingModel::Constant);
    materialGround->setCullMode(VROCullMode::None);
    
    std::shared_ptr<VRONode> groundNode = std::make_shared<VRONode>();
    groundNode->setGeometry(groundBox);
    groundNode->setPosition({0, -10, -5});
    groundNode->setTag("GROUND");
    rootNode->addChildNode(groundNode);
    
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    groundNode->setEventDelegate(_eventDelegate);
    
    std::shared_ptr<VROPhysicsBody> physicsGround = groundNode->initPhysicsBody(VROPhysicsBody::VROPhysicsBodyType::Static,
                                                                                0, nullptr);
    std::shared_ptr<VROPhysicsWorld> physicsWorld = scene->getPhysicsWorld();
    physicsWorld->setGravity({0, -9.81f, 0});
    physicsWorld->addPhysicsBody(physicsGround);
    physicsGround->setRestitution(0);
    
    _rootNode = rootNode;
}

void VROPhysicsEventDelegate::onClick(int source, ClickState clickState, std::vector<float> position) {
    if (clickState == Clicked) {
        _test->createPhysicsBox({ position[0], 10, position[2] }, "box3");
    }
}

std::shared_ptr<VRONode> VROPhysicsTest::createPhysicsBox(VROVector3f position, std::string tag) {
    std::shared_ptr<VROBox> box = VROBox::createBox(1, 1, 1);
    box->setName("Box 1");
    std::shared_ptr<VROMaterial> material = box->getMaterials()[0];
    material->getDiffuse().setColor({0.6, 0.3, 0.3, 0.5});
    material->setLightingModel(VROLightingModel::Constant);
    material->setCullMode(VROCullMode::None);
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setTag(tag);
    boxNode->setPosition(position);
    _rootNode->addChildNode(boxNode);
    std::shared_ptr<VROPhysicsBody> physicsBody = boxNode->initPhysicsBody(VROPhysicsBody::VROPhysicsBodyType::Dynamic, 5,
                                                                           nullptr);
    physicsBody->setRestitution(0);
    physicsBody->setUseGravity(true);
    
    std::shared_ptr<VROPhysicsWorld> physicsWorld = _sceneController->getScene()->getPhysicsWorld();
    physicsWorld->addPhysicsBody(physicsBody);
    
    return boxNode;
}

