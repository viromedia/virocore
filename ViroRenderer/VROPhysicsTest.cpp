//
//  VROPhysicsTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROPhysicsTest.h"
#include "VROTestUtil.h"

VROPhysicsTest::VROPhysicsTest() :
    VRORendererTest(VRORendererTestType::Physics) {
        
}

VROPhysicsTest::~VROPhysicsTest() {
    
}

void VROPhysicsTest::build(std::shared_ptr<VRORenderer> renderer,
                           std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
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
    physicsGround->setRestitution(1.0);
    
    std::shared_ptr<VROPhysicsWorld> physicsWorld = scene->getPhysicsWorld();
    physicsWorld->setGravity({0, -9.81f, 0});
    
    _rootNode = rootNode;
}

void VROPhysicsEventDelegate::onClick(int source, std::shared_ptr<VRONode> node,
                                      ClickState clickState, std::vector<float> position) {
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
    physicsBody->setRestitution(1.0);
    physicsBody->setUseGravity(true);
    return boxNode;
}

