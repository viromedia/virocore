//
//  VROVideoSphereTest.cpp
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

#include "VROVideoSphereTest.h"
#include "VROTestUtil.h"

VROVideoSphereTest::VROVideoSphereTest() :
    VRORendererTest(VRORendererTestType::VideoSphere) {
        
}

VROVideoSphereTest::~VROVideoSphereTest() {
    
}

void VROVideoSphereTest::build(std::shared_ptr<VRORenderer> renderer,
                               std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
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
