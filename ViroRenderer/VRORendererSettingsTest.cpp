//
//  VRORendererSettingsTest.cpp
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

#include "VRORendererSettingsTest.h"
#include "VROTestUtil.h"
#include "VROChoreographer.h"
#include "VRORenderer.h"

VRORendererSettingsTest::VRORendererSettingsTest() :
    VRORendererTest(VRORendererTestType::Bloom) {
        
}

VRORendererSettingsTest::~VRORendererSettingsTest() {
    
}

void VRORendererSettingsTest::build(std::shared_ptr<VRORenderer> renderer,
                                    std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                    std::shared_ptr<VRODriver> driver) {
    _index = 0;
    _renderer = renderer;
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    scene->getRootNode()->setCamera(camera);
    _pointOfView = scene->getRootNode();
    
    camera->setPosition({ 2, 2, 2 });
    camera->setBaseRotation({ 0, M_PI_2 / 2, 0 });
    
    std::vector<VROVector3f> lightPositions;
    lightPositions.push_back({ -2.0f, 1.5f, 1.5f});
    lightPositions.push_back({-4.0f, 1.8f, -3.0f});
    lightPositions.push_back({ 3.0f, 1.2f,  1.0f});
    lightPositions.push_back({.8f,  1.4f, -1.0f});
    
    std::vector<VROVector3f> lightColors;
    lightColors.push_back({5.0, 5.0, 5.0});
    lightColors.push_back({5.0, 0.0, 0.0});
    lightColors.push_back({0.0, 5.0, 0.0});
    lightColors.push_back({0.0, 0.0, 5.0});
    
    for (int i = 0; i < lightPositions.size(); i++) {
        std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
        light->setColor(lightColors[i]);
        light->setPosition({lightPositions[i].x, lightPositions[i].y - 0.3f, lightPositions[i].z});
        light->setDirection({0, -1, 0});
        light->setAttenuationStartDistance(10);
        light->setAttenuationEndDistance(20);
        light->setSpotInnerAngle(30);
        light->setSpotOuterAngle(60);
        light->setCastsShadow(true);
        light->setIntensity(2000);
        rootNode->addLight(light);
    }
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setIntensity(200);
    rootNode->addLight(ambient);
    
    std::shared_ptr<VROTexture> woodTexture = VROTestUtil::loadDiffuseTexture("wood");
    woodTexture->setWrapS(VROWrapMode::Repeat);
    woodTexture->setWrapT(VROWrapMode::Repeat);
    woodTexture->setMinificationFilter(VROFilterMode::Linear);
    woodTexture->setMagnificationFilter(VROFilterMode::Linear);
    woodTexture->setMipFilter(VROFilterMode::Linear);
    
    std::vector<VROVector3f> boxPositions;
    boxPositions.push_back({ 0,  -2, 0 });
    boxPositions.push_back({ -2.0f, 0.5f, 1.5f});
    boxPositions.push_back({-4.0f,  0.8f, -3.0f});
    boxPositions.push_back({ 3.0f, 0.2f,  1.0f});
    boxPositions.push_back({.8f,  0.4f, -1.0f});
    
    std::vector<VROVector3f> boxScales;
    boxScales.push_back({12.5, 0.5, 12.5});
    boxScales.push_back({0.1, 0.1, 0.1});
    boxScales.push_back({0.1, 0.1, 0.1});
    boxScales.push_back({0.1, 0.1, 0.1});
    boxScales.push_back({0.1, 0.1, 0.1});
    
    for (int i = 0; i < boxPositions.size(); i++) {
        std::shared_ptr<VROBox> box = VROBox::createBox(2, 2, 2);
        
        std::shared_ptr<VROMaterial> boxMaterial = box->getMaterials()[0];
        boxMaterial->setLightingModel(VROLightingModel::PhysicallyBased);
        boxMaterial->getDiffuse().setTexture(woodTexture);
        
        std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
        boxNode->setGeometry(box);
        boxNode->setPosition({ boxPositions[i].x, boxPositions[i].y, boxPositions[i].z });
        boxNode->setScale(boxScales[i]);
        rootNode->addChildNode(boxNode);
    }
    
    std::vector<std::shared_ptr<VROMaterial>> boxMaterials;
    for (int i = 0; i < lightPositions.size(); i++) {
        std::shared_ptr<VROBox> box = VROBox::createBox(2, 2, 2);
        
        std::shared_ptr<VROMaterial> boxMaterial = box->getMaterials()[0];
        boxMaterial->setLightingModel(VROLightingModel::Constant);
        boxMaterial->getDiffuse().setColor({lightColors[i].x, lightColors[i].y, lightColors[i].z, 1});
        boxMaterial->setBloomThreshold(0.5);
        
        std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
        boxNode->setGeometry(box);
        boxNode->setPosition({ lightPositions[i].x, lightPositions[i].y, lightPositions[i].z });
        boxNode->setScale({ 0.25, 0.25, 0.25 });
        rootNode->addChildNode(boxNode);
        
        boxMaterials.push_back(boxMaterial);
    }
    
    _eventDelegate = std::make_shared<VRORendererSettingsEventDelegate>(this);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    rootNode->setEventDelegate(_eventDelegate);
}

void VRORendererSettingsTest::changeSettings() {
    std::shared_ptr<VROChoreographer> choreographer = _renderer->getChoreographer();
    choreographer->setShadowsEnabled(true);
    choreographer->setHDREnabled(true);
    choreographer->setPBREnabled(true);
    choreographer->setBloomEnabled(true);
    
    switch (_index) {
        case 0:
            pinfo("Disabling shadows");
            choreographer->setShadowsEnabled(false);
            break;
        case 1:
            pinfo("Disabling HDR");
            choreographer->setHDREnabled(false);
            break;
        case 2:
            pinfo("Disabling PBR");
            choreographer->setPBREnabled(false);
            break;
        case 3:
            pinfo("Disabling Bloom");
            choreographer->setBloomEnabled(false);
            break;
        case 4:
            pinfo("Enabling All");
        default:
            break;
    }
    
    _index = (_index + 1) % 5;
}

void VRORendererSettingsEventDelegate::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState,
                                  std::vector<float> position) {
    if (clickState == ClickState::Clicked) {
        _test->changeSettings();
    }
}
