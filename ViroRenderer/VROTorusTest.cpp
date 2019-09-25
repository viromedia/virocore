//
//  VROTorusTest.cpp
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

#include "VROTorusTest.h"
#include "VROTestUtil.h"

VROTorusTest::VROTorusTest() :
    VRORendererTest(VRORendererTestType::Torus) {
        
}

VROTorusTest::~VROTorusTest() {
    
}

void VROTorusTest::build(std::shared_ptr<VRORenderer> renderer,
                         std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
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

