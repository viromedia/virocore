//
//  VROBodyMesherTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 5/23/19.
//  Copyright © 2019 Viro Media. All rights reserved.
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

#include "VROBodyMesherTest.h"
#include "VROTestUtil.h"
#include "VROSphere.h"
#include "VROPoseFilterMovingAverage.h"
#include "VROPoseFilterLowPass.h"

#if VRO_PLATFORM_IOS
#include "VROBodyMesheriOS.h"
#include "VRODriverOpenGLiOS.h"
#include "VROVideoTexture.h"
#endif

VROBodyMesherTest::VROBodyMesherTest() :
    VRORendererTest(VRORendererTestType::BodyMesher) {
    
}

VROBodyMesherTest::~VROBodyMesherTest() {
    
}

void VROBodyMesherTest::build(std::shared_ptr<VRORenderer> renderer,
                              std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                              std::shared_ptr<VRODriver> driver) {
    _renderer = renderer;
    _sceneController = std::make_shared<VROARSceneController>();
    _sceneController->setDelegate(shared_from_this());
    
    _arScene = std::dynamic_pointer_cast<VROARScene>(_sceneController->getScene());
    _arScene->initDeclarativeSession();
    
#if VRO_PLATFORM_IOS
    VROViewAR *view = (VROViewAR *) std::dynamic_pointer_cast<VRODriverOpenGLiOS>(driver)->getView();
    _bodyMesher = std::make_shared<VROBodyMesheriOS>();
    _surfaceRenderer = std::make_shared<VROBodySurfaceRenderer>(view, _bodyMesher);

    _bodyMesher->setDelegate(shared_from_this());
    _bodyMesher->initBodyTracking(view.cameraPosition, driver);
    _bodyMesher->startBodyTracking();
#endif
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    _arScene->getRootNode()->addLight(ambient);
}

void VROBodyMesherTest::onBodyMeshUpdated(const std::vector<float> &vertices, std::shared_ptr<VROGeometry> mesh) {
#if VRO_PLATFORM_IOS
    if (true) {
        _surfaceRenderer->onBodyMeshUpdated(vertices, mesh);
        return;
    }
#endif
    if (!_bodyMeshNode) {
        _bodyMeshNode = std::make_shared<VRONode>();
        _bodyMeshNode->setPosition({ 0, -1.0, -2 });
        //_bodyMeshNode->setRotationEuler({ 0, M_PI_2, 0 });
        _arScene->getRootNode()->addChildNode(_bodyMeshNode);
        
        VROTransaction::begin();
        VROTransaction::setAnimationDuration(10);
        //_bodyMeshNode->setRotationEulerY(M_PI - 0.0001);
        
        VROTransaction::commit();
    }
    
    if (!_bodyMesh) {
        _bodyMesh = mesh;
        _bodyMeshNode->setGeometry(_bodyMesh);
    }
}

