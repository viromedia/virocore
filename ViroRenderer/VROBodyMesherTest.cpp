//
//  VROBodyMesherTest.cpp
//  ViroKit
//
//  Created by Raj Advani on 5/23/19.
//  Copyright © 2019 Viro Media. All rights reserved.
//

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

    _bodyMesher->setDelegate(shared_from_this());
    _bodyMesher->initBodyTracking(view.cameraPosition, driver);
    _bodyMesher->startBodyTracking();
#endif
    
    std::shared_ptr<VROLight> ambient = std::make_shared<VROLight>(VROLightType::Ambient);
    ambient->setColor({ 0.6, 0.6, 0.6 });
    _arScene->getRootNode()->addLight(ambient);
}

void VROBodyMesherTest::onBodyMeshUpdated() {
    
}

