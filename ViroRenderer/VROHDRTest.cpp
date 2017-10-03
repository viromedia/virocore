//
//  VROHDRTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROHDRTest.h"
#include "VROTestUtil.h"

VROHDRTest::VROHDRTest() :
    VRORendererTest(VRORendererTestType::HDR) {
        
}

VROHDRTest::~VROHDRTest() {
    
}

void VROHDRTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                       std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
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
        rootNode->addLight(light);
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
        rootNode->addChildNode(surfaceNode);
    }
}
