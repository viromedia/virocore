//
//  VROVideoSphereTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROVideoSphereTest.h"
#include "VROTestUtil.h"

VROVideoSphereTest::VROVideoSphereTest() :
    VRORendererTest(VRORendererTestType::VideoSphere) {
        
}

VROVideoSphereTest::~VROVideoSphereTest() {
    
}

void VROVideoSphereTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                               std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    
    std::shared_ptr<VROLight> light = std::make_shared<VROLight>(VROLightType::Spot);
    light->setColor({ 1.0, 0.9, 0.9 });
    light->setPosition( { 0, 0, 0 });
    light->setDirection( { 0, 0, -1.0 });
    light->setAttenuationStartDistance(5);
    light->setAttenuationEndDistance(10);
    light->setSpotInnerAngle(0);
    light->setSpotOuterAngle(20);
    
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    rootNode->setPosition({0, 0, 0});
    rootNode->addLight(light);
    
    std::string url = VROTestUtil::getURLForResource("surfing", "mp4");
    
    _videoTexture = VROTestUtil::loadVideoTexture(driver, [url, rootNode, frameSynchronizer, driver] (std::shared_ptr<VROVideoTexture> texture) {
        texture->loadVideo(url, frameSynchronizer, driver);
        texture->play();
        rootNode->setBackgroundSphere(texture);
    });
}
