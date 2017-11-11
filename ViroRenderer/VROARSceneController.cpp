//
//  VROARSceneController.cpp
//  ViroKit
//
//  Created by Raj Advani on 11/11/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROARSceneController.h"

VROARSceneController::VROARSceneController() {
    std::shared_ptr<VROARScene> scene = std::make_shared<VROARScene>();
    _scene = scene;
    _scene->getRootNode()->setScene(_scene, true);
}

VROARSceneController::~VROARSceneController() {
    
}

void VROARSceneController::onSceneWillAppear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    VROSceneController::onSceneWillAppear(context, driver);
    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_scene);
    if (arScene) {
        arScene->willAppear();
    }
}

void VROARSceneController::onSceneWillDisappear(VRORenderContext *context, std::shared_ptr<VRODriver> driver) {
    VROSceneController::onSceneWillDisappear(context, driver);
    std::shared_ptr<VROARScene> arScene = std::dynamic_pointer_cast<VROARScene>(_scene);
    if (arScene) {
        arScene->willDisappear();
    }
}
