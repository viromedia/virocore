//
//  VROPBRTexturedTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 1/18/18.
//  Copyright © 2018 Viro Media. All rights reserved.
//

#include "VROPBRTexturedTest.h"
#include "VROTestUtil.h"
#include "VROCompress.h"
#include "VROSphere.h"

VROPBRTexturedTest::VROPBRTexturedTest() :
    VRORendererTest(VRORendererTestType::PBRTextured) {
        _angle = 0;
}

VROPBRTexturedTest::~VROPBRTexturedTest() {
    
}

void VROPBRTexturedTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer, std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    
    VROVector3f lightPositions[] = {
        VROVector3f(0, 0, 10),
    };
    float intensity = 150;
    VROVector3f lightColors[] = {
        VROVector3f(1, 1, 1),
    };
    
    int rows = 7;
    int columns = 7;
    float spacing = 2.5;
    
    std::shared_ptr<VROTexture> albedo = VROTestUtil::loadTexture("pbr_sample_albedo", true);
    std::shared_ptr<VROTexture> roughness = VROTestUtil::loadTexture("pbr_sample_roughness", false);
    std::shared_ptr<VROTexture> metalness = VROTestUtil::loadTexture("pbr_sample_metallic", false);
    std::shared_ptr<VROTexture> normal = VROTestUtil::loadTexture("pbr_sample_normal", false);
    std::shared_ptr<VROTexture> ao = VROTestUtil::loadTexture("pbr_sample_ao", true);
    
    std::shared_ptr<VRONode> sphereContainerNode = std::make_shared<VRONode>();
    
    // Render an array of spheres, varying roughness and metalness
    for (int row = 0; row < rows; ++row) {
        float radius = 1;

        for (int col = 0; col < columns; ++col) {
            std::shared_ptr<VROSphere> sphere = VROSphere::createSphere(radius, 20, 20, true);
            const std::shared_ptr<VROMaterial> &material = sphere->getMaterials().front();
            material->getDiffuse().setTexture(albedo);
            material->getRoughness().setTexture(roughness);
            material->getMetalness().setTexture(metalness);
            material->getNormal().setTexture(normal);
            material->getAmbientOcclusion().setTexture(ao);
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
    
    // Render the light sources as spheres as well
    for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i) {
        std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Omni);
        light->setColor(lightColors[i]);
        light->setPosition(lightPositions[i]);
        light->setAttenuationStartDistance(20);
        light->setAttenuationEndDistance(30);
        light->setIntensity(intensity);
        rootNode->addLight(light);
    }
    
    std::shared_ptr<VRONodeCamera> camera = std::make_shared<VRONodeCamera>();
    std::shared_ptr<VRONode> cameraNode = std::make_shared<VRONode>();
    cameraNode->setPosition({ 0, 0, 3 });
    cameraNode->setCamera(camera);
    rootNode->addChildNode(cameraNode);
    
    std::shared_ptr<VROAction> action = VROAction::perpetualPerFrameAction([this] (VRONode *const node, float seconds) {
        _angle += .015;
        node->setRotation({ 0, _angle, 0});
        return true;
    });
    //sphereContainerNode->runAction(action);
    
    _pointOfView = cameraNode;
}

