//
//  VROBloomTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROBloomTest.h"
#include "VROTestUtil.h"

VROBloomTest::VROBloomTest() :
    VRORendererTest(VRORendererTestType::Bloom) {
        
}

VROBloomTest::~VROBloomTest() {
    
}

void VROBloomTest::build(std::shared_ptr<VRORenderer> renderer,
                         std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                         std::shared_ptr<VRODriver> driver) {
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
        std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Omni);
        light->setColor(lightColors[i]);
        light->setPosition({ lightPositions[i].x, lightPositions[i].y, lightPositions[i].z});
        light->setAttenuationStartDistance(0);
        light->setAttenuationEndDistance(5);
        rootNode->addLight(light);
    }
    
    std::shared_ptr<VROTexture> woodTexture = VROTestUtil::loadDiffuseTexture("wood");
    woodTexture->setWrapS(VROWrapMode::Repeat);
    woodTexture->setWrapT(VROWrapMode::Repeat);
    woodTexture->setMinificationFilter(VROFilterMode::Linear);
    woodTexture->setMagnificationFilter(VROFilterMode::Linear);
    woodTexture->setMipFilter(VROFilterMode::Linear);
    
    std::vector<VROVector3f> boxPositions;
    boxPositions.push_back({ 0,  -2, 0 });
    boxPositions.push_back({ 0, 4.5, 0 });
    boxPositions.push_back({ 2, 0, 1 });
    boxPositions.push_back({ 3, -1, 2 });
    boxPositions.push_back({ 0, 2.7, 4 });
    boxPositions.push_back({ -2, -1, -3 });
    boxPositions.push_back({ -3, 0, 0 });
    
    std::vector<VROVector3f> boxScales;
    boxScales.push_back({12.5, 0.5, 12.5});
    boxScales.push_back({0.5, 0.5, 0.5});
    boxScales.push_back({0.5, 0.5, 0.5});
    boxScales.push_back({1, 1, 1});
    boxScales.push_back({1.25, 1.25, 1.25});
    boxScales.push_back({1, 1, 1});
    boxScales.push_back({0.5, 0.5, 0.5});
    
    std::vector<VROQuaternion> boxRotations;
    boxRotations.push_back({0, 1, 0, 0});
    boxRotations.push_back({0, 1, 0, 0});
    boxRotations.push_back({0, 1, 0, 0});
    boxRotations.push_back({1, 0, 1, toRadians(60)});
    boxRotations.push_back({1, 0, 1, toRadians(23)});
    boxRotations.push_back({1, 0, 1, toRadians(145)});
    boxRotations.push_back({0, 1, 0, 0});
    
    for (int i = 0; i < boxPositions.size(); i++) {
        std::shared_ptr<VROBox> box = VROBox::createBox(2, 2, 2);
        
        std::shared_ptr<VROMaterial> boxMaterial = box->getMaterials()[0];
        boxMaterial->setLightingModel(VROLightingModel::Lambert);
        boxMaterial->getDiffuse().setTexture(woodTexture);
        
        std::shared_ptr<VRONode> boxNode = std::make_shared<VRONode>();
        boxNode->setGeometry(box);
        boxNode->setPosition({ boxPositions[i].x, boxPositions[i].y, boxPositions[i].z });
        boxNode->setScale(boxScales[i]);
        //boxNode->setRotation(boxRotations[i]);
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
}
