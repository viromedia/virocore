//
//  VROPhotometricLightTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
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

#include "VROPhotometricLightTest.h"
#include "VROTestUtil.h"
#include "VROSurface.h"
#include "VROCompress.h"
#include "VROSphere.h"
#include "VROPortal.h"

VROPhotometricLightTest::VROPhotometricLightTest() :
    VRORendererTest(VRORendererTestType::PBRDirect) {
    _angle = 0;
    _textureIndex = 0;
}

VROPhotometricLightTest::~VROPhotometricLightTest() {
    
}

void VROPhotometricLightTest::build(std::shared_ptr<VRORenderer> renderer,
                                    std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                    std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    
    std::shared_ptr<VROSurface> plane = VROSurface::createSurface(50, 50);
    std::shared_ptr<VROMaterial> material = plane->getMaterials()[0];
    material->getDiffuse().setColor({ 1.0, 1.0, 1.0, 1.0 });
    material->setLightingModel(VROLightingModel::PhysicallyBased);
    
    std::shared_ptr<VRONode> planeNode = std::make_shared<VRONode>();
    planeNode->setGeometry(plane);
    planeNode->setPosition({0, 0, 0});
    rootNode->addChildNode(planeNode);
    
    std::shared_ptr<VROGeometry> box = VROBox::createBox(1, 1, 1);
    box->getMaterials().front()->getDiffuse().setColor({0.0, 0.0, 1.0, 1.0});
    
    std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
    boxNode->setGeometry(box);
    boxNode->setPosition({2, 0, 0});
    rootNode->addChildNode(boxNode);
    
    std::shared_ptr<VRONode> lightNode = std::make_shared<VRONode>();
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setIntensity(250);
    light->setTemperature(2000);
    light->setAttenuationStartDistance(10);
    light->setAttenuationEndDistance(100);
    light->setSpotInnerAngle(45);
    light->setSpotOuterAngle(90);
    //light->setCastsShadow(true);
    //light->setColor({1.0, 0.0, 0.0});
    light->setDirection({0, 0, -1});
    lightNode->setPosition({0, 0, 5});
    lightNode->addLight(light);
    rootNode->addChildNode(lightNode);
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({0.2, 0.2, 0.2});
    //rootNode->addLight(ambient);
    
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    
    std::shared_ptr<VRONode> cameraNode = std::make_shared<VRONode>();
    cameraNode->setPosition({0, 0, 8});
    cameraNode->setCamera(camera);
    
    _pointOfView = cameraNode;
}

void VROPhotometricLightEventDelegate::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState,
                                               std::vector<float> position) {
    if (clickState != Clicked) {
        return;
    }
}

