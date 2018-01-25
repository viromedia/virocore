//
//  VRODiffuseIrradianceTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VRODiffuseIrradianceTest.h"
#include "VROTestUtil.h"
#include "VROCompress.h"
#include "VROSphere.h"

VRODiffuseIrradianceTest::VRODiffuseIrradianceTest() :
    VRORendererTest(VRORendererTestType::PBRDirect) {
    _angle = 0;
    _textureIndex = 0;
}

VRODiffuseIrradianceTest::~VRODiffuseIrradianceTest() {
    
}

void VRODiffuseIrradianceTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer, std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    nextEnvironment();
    
    VROVector3f lightPositions[] = {
        VROVector3f(-10,  10, 10),
        VROVector3f( 10,  10, 10),
        VROVector3f(-10, -10, 10),
        VROVector3f( 10, -10, 10),
    };
    VROVector3f lightColors[] = {
        VROVector3f(300.0f, 300.0f, 300.0f),
        VROVector3f(300.0f, 300.0f, 300.0f),
        VROVector3f(300.0f, 300.0f, 300.0f),
        VROVector3f(300.0f, 300.0f, 300.0f),
    };
    
    int rows = 7;
    int columns = 7;
    float spacing = 2.5;
    
    std::shared_ptr<VRONode> sphereContainerNode = std::make_shared<VRONode>();
    
    // Render an array of spheres, varying roughness and metalness
    for (int row = 0; row < rows; ++row) {
        float radius = 1;
        float metalness = (float) row / (float) rows;
        
        for (int col = 0; col < columns; ++col) {
            // Clamp the roughness to [0.05, 1.0] as perfectly smooth surfaces (roughness of 0.0)
            // tend to look off on direct lighting
            float roughness = VROMathClamp((float) col / (float) columns, 0.05, 1.0);
            
            std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(radius, 20, 20, true);
            const std::shared_ptr<VROMaterial> &material = sphere->getMaterials().front();
            material->getDiffuse().setColor({ 0.5, 0.5, 0.5, 1.0 });
            material->getRoughness().setColor({ roughness, 1.0, 1.0, 1.0 });
            material->getMetalness().setColor({ metalness, 1.0, 1.0, 1.0 });
            material->getAmbientOcclusion().setColor({ 1.0, 1.0, 1.0, 1.0 });
            material->setLightingModel(VROLightingModel::PhysicallyBased);
            
            std::shared_ptr<VRONode> sphereNode = std::make_shared<VRONode>();
            sphereNode->setPosition({ (float)(col - (columns / 2)) * spacing,
                                      (float)(row - (rows    / 2)) * spacing,
                                      0.0f });
            sphereNode->setGeometry(sphere);
            sphereContainerNode->addChildNode(sphereNode);
        }
    }
    
    rootNode->addChildNode(sphereContainerNode);
    
    _eventDelegate = std::make_shared<VROIBLEventDelegate>(scene, this);
    _eventDelegate->setEnabledEvent(VROEventDelegate::EventAction::OnClick, true);
    sphereContainerNode->setEventDelegate(_eventDelegate);
    
    // Render the light sources as spheres as well
    for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) {
        std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Omni);
        light->setColor(lightColors[i]);
        light->setPosition(lightPositions[i]);
        light->setAttenuationStartDistance(20);
        light->setAttenuationEndDistance(30);
        rootNode->addLight(light);
    }
    
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    std::shared_ptr<VRONode> cameraNode = std::make_shared<VRONode>();
    cameraNode->setPosition({ 0, 0, 9 });
    cameraNode->setCamera(camera);
    rootNode->addChildNode(cameraNode);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([this] (VRONode *const node, float seconds) {
        _angle += .0015;
        node->setRotation({ 0, _angle, 0});
        return true;
    });
    //sphereContainerNode->runAction(action);
    
    _pointOfView = cameraNode;
}

void VRODiffuseIrradianceTest::nextEnvironment() {
    std::shared_ptr<VROTexture> environment;
    if (_textureIndex == 0) {
        environment = VROTestUtil::loadRadianceHDRTexture("newport_loft");
    }
    else if (_textureIndex == 1) {
        environment = VROTestUtil::loadRadianceHDRTexture("Mans_Outside_2k");
    }
    else if (_textureIndex == 2) {
        environment = VROTestUtil::loadRadianceHDRTexture("Ridgecrest_Road_Ref");
    }
    else if (_textureIndex == 3) {
        environment = VROTestUtil::loadRadianceHDRTexture("WoodenDoor_Ref");
    }
    
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    scene->getRootNode()->setLightingEnvironment(environment);
    scene->getRootNode()->setBackgroundSphere(environment);
    
    _textureIndex = (_textureIndex + 1) % 4;
}

void VROIBLEventDelegate::onClick(int source, std::shared_ptr<VRONode> node, ClickState clickState,
                                  std::vector<float> position) {
    if (clickState != Clicked) {
        return;
    }
    _test->nextEnvironment();
}

