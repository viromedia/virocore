//
//  VROHDRTest.cpp
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

#include "VROHDRTest.h"
#include "VROTestUtil.h"
#include "VROToneMappingRenderPass.h"

VROHDRTest::VROHDRTest() :
    VRORendererTest(VRORendererTestType::HDR),
    _activeScene(0) {
        
}

VROHDRTest::~VROHDRTest() {
    
}

std::shared_ptr<VRONode> VROHDRTest::buildBoxScene() {
    _sceneController->getScene()->getRootNode()->removeBackground();

    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    VROVector3f lightPositions[4] = {
        {  0.0,  0.0, -49.5 },
        { -1.4, -1.9, -9.0 },
        {  0.0, -1.8, -4.0 },
        {  0.8, -1.7, -6.0 },
    };
    VROVector3f lightColors[4] = {
        { 200, 200, 200 },
        { 0.1, 0.0, 0.0 },
        { 0.0, 0.0, 0.2 },
        { 0.0, 0.1, 0.0 },
    };
    
    for (int i = 0; i < 4; i++) {
        std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Omni);
        light->setColor(lightColors[i]);
        light->setPosition(lightPositions[i]);
        light->setAttenuationStartDistance(0);
        light->setAttenuationEndDistance(40);
        sceneNode->addLight(light);
    }
    
    std::shared_ptr<VROTexture> woodTexture = VROTestUtil::loadDiffuseTexture("wood");
    woodTexture->setWrapS(VROWrapMode::Repeat);
    woodTexture->setWrapT(VROWrapMode::Repeat);
    woodTexture->setMinificationFilter(VROFilterMode::Linear);
    woodTexture->setMagnificationFilter(VROFilterMode::Linear);
    woodTexture->setMipFilter(VROFilterMode::Linear);
    
    /*
     Create 5 surfaces surrounding the user.
     */
    VROVector3f surfaceRotation[5] = {
        { 0, M_PI_2, 0},
        { 0, -M_PI_2, 0 },
        { M_PI_2, 0, 0},
        { -M_PI_2, 0, 0},
        { 0, 0, 0 },
    };
    
    float width = 2.5;
    VROVector3f surfacePosition[5] = {
        { -width, 0, 0 },
        {  width, 0, 0 },
        {  0, width, 0 },
        {  0, -width, 0},
        { 0, 0, -52.5 }
    };
    
    for (int i = 0; i < 5; i++) {
        std::shared_ptr<VROSurface> surface = VROSurface::createSurface(40, 40);
        surface->setName("Surface");
        surface->getMaterials().front()->setLightingModel(VROLightingModel::Lambert);
        surface->getMaterials().front()->getDiffuse().setTexture(woodTexture);
        
        std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
        surfaceNode->setGeometry(surface);
        surfaceNode->setRotationEuler(surfaceRotation[i]);
        surfaceNode->setPosition(surfacePosition[i]);
        surfaceNode->setOpacity(1.0);
        sceneNode->addChildNode(surfaceNode);
    }
    return sceneNode;
}

std::shared_ptr<VRONode> VROHDRTest::buildWoodenDoorScene() {
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    _sceneController->getScene()->getRootNode()->setBackgroundSphere(VROTestUtil::loadHDRTexture("wooden"));
    return sceneNode;
}

std::shared_ptr<VRONode> VROHDRTest::buildPlayaScene() {
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    _sceneController->getScene()->getRootNode()->setBackgroundSphere(VROTestUtil::loadHDRTexture("sunrise"));
    return sceneNode;
}

std::shared_ptr<VRONode> VROHDRTest::buildIndoorScene() {
    std::shared_ptr<VRONode> sceneNode = std::make_shared<VRONode>();
    _sceneController->getScene()->getRootNode()->setBackgroundSphere(VROTestUtil::loadDiffuseTexture("interior_viro.jpg", VROMipmapMode::None));
    return sceneNode;
}

void VROHDRTest::build(std::shared_ptr<VRORenderer> renderer,
                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
    changeScene();
    
    _eventDelegate = std::make_shared<VROHDREventDelegate>(this);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    scene->getRootNode()->setEventDelegate(_eventDelegate);
}

void VROHDRTest::changeScene() {
    _activeScene = (_activeScene + 1) % 4;
    
    std::shared_ptr<VRONode> rootNode = _sceneController->getScene()->getRootNode();
    std::vector<std::shared_ptr<VRONode>> children = rootNode->getChildNodes();
    if (children.size() > 0) {
        children[0]->removeFromParentNode();
    }
    
    std::shared_ptr<VRONode> sceneNode;
    if (_activeScene == 0) {
        sceneNode = buildBoxScene();
    }
    else if (_activeScene == 1) {
        sceneNode = buildWoodenDoorScene();
    }
    else if (_activeScene == 2) {
        sceneNode = buildPlayaScene();
    }
    else {
        sceneNode = buildIndoorScene();
    }
    rootNode->addChildNode(sceneNode);
}

void VROHDREventDelegate::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState,
                                  std::vector<float> position) {
    if (clickState == ClickState::Clicked) {
        _test->changeScene();
    }
}
