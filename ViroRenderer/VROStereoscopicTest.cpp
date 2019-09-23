//
//  VROStereoscopicTest.cpp
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

#include "VROStereoscopicTest.h"
#include "VROTestUtil.h"

VROStereoscopicTest::VROStereoscopicTest() :
    VRORendererTest(VRORendererTestType::Stereoscopic) {
        
}

VROStereoscopicTest::~VROStereoscopicTest() {
    
}

void VROStereoscopicTest::build(std::shared_ptr<VRORenderer> renderer,
                                std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
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
