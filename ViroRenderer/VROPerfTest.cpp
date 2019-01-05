//
//  VROPerfTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROPerfTest.h"
#include "VROTestUtil.h"

VROPerfTest::VROPerfTest() :
    VRORendererTest(VRORendererTestType::Perf) {
        
}

VROPerfTest::~VROPerfTest() {
    
}

void VROPerfTest::build(std::shared_ptr<VRORenderer> renderer,
                        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                        std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROARSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 1.0, 1.0, 1.0 });
    ambient->setIntensity(600);
    ambient->setInfluenceBitMask(0xFFFFFFFF);
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(ambient);
    
    std::vector<VROVector3f> positions;
    positions.push_back({ -7, -3, -6 });
    positions.push_back({ -5, -3, -3 });
    positions.push_back({ -3, -3, -6 });
    positions.push_back({ -1, -3, -3 });
    positions.push_back({  1, -3, -6 });
    positions.push_back({  3, -3, -3 });
    positions.push_back({  5, -3, -6 });
    positions.push_back({  7, -3, -3 });
    positions.push_back({  9, -3, -6 });
    positions.push_back({ 11, -3, -3 });
    positions.push_back({ -5, -3, -9 });
    positions.push_back({ -7, -3, -1 });
    positions.push_back({ -1, -3, -9 });
    positions.push_back({ -3, -3, -1 });
    positions.push_back({  3, -3, -9 });
    positions.push_back({  1, -3, -1 });
    positions.push_back({  7, -3, -9 });
    positions.push_back({  5, -3, -1 });
    positions.push_back({ 11, -3, -9 });
    positions.push_back({  9, -3, -1 });
    
    int i = 0;
    for (VROVector3f position : positions) {
        std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
        light->setColor({ 1.0, 1.0, 1.0 });
        light->setPosition({ position.x, 5, position.z });
        light->setDirection({ 0, -1.0, 0 });
        light->setAttenuationStartDistance(50);
        light->setAttenuationEndDistance(75);
        light->setSpotInnerAngle(70);
        light->setSpotOuterAngle(120);
        light->setCastsShadow(true);
        light->setInfluenceBitMask(0x1 << i);
        rootNode->addLight(light);
        
        std::shared_ptr<VRONode> fbx = VROTestUtil::loadFBXModel("pug", position, { 1, 1, 1 }, { 0, 0, 0 },
                                                                 (0x1 << i), "Take 001", driver, nullptr);
        rootNode->addChildNode(fbx);
        
        std::shared_ptr<VROSurface> surface = VROSurface::createSurface(2, 2);
        surface->setName("Surface");
        surface->getMaterials().front()->setLightingModel(VROLightingModel::Lambert);
        //VROARShadow::apply(surface->getMaterials().front());
        
        std::shared_ptr<VRONode> surfaceNode = std::make_shared<VRONode>();
        surfaceNode->setGeometry(surface);
        surfaceNode->setRotationEuler({ -M_PI_2, 0, 0 });
        surfaceNode->setPosition({position.x, -3, position.z});
        VROTestUtil::setLightMasks(surfaceNode, 0x1 << i);
        rootNode->addChildNode(surfaceNode);
        
        pinfo("Set light mask %d to %d", i, (0x1 << i));
        ++i;
    }
}
