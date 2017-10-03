//
//  VROStereoscopicTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 10/1/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROStereoscopicTest.h"
#include "VROTestUtil.h"

VROStereoscopicTest::VROStereoscopicTest() :
    VRORendererTest(VRORendererTestType::Stereoscopic) {
        
}

VROStereoscopicTest::~VROStereoscopicTest() {
    
}

void VROStereoscopicTest::build(std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                std::shared_ptr<VRODriver> driver) {
    _sceneController = std::make_shared<VROSceneController>();
    std::shared_ptr<VROScene> scene = _sceneController->getScene();
    std::shared_ptr<VROPortal> rootNode = scene->getRootNode();
    
    // Debug toggle between stereo image and stereo video background
    bool showImage = false;
    if (showImage) {
        std::shared_ptr<VROTexture> imgTexture = VROTestUtil::loadDiffuseTexture("stereo3601.jpg", VROMipmapMode::None,
                                                                                 VROStereoMode::BottomTop);
        rootNode->setBackgroundSphere(imgTexture);
    }
    else {
        std::string url = VROTestUtil::getURLForResource("stereoVid360", "mp4");
    
        _videoTexture = VROTestUtil::loadVideoTexture(driver, [url, rootNode, frameSynchronizer, driver] (std::shared_ptr<VROVideoTexture> texture) {
            texture->loadVideo(url, frameSynchronizer, driver);
            texture->play();
            rootNode->setBackgroundSphere(texture);
        }, VROStereoMode::BottomTop);
    }
}
