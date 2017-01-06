//
//  VROSceneRendererOVR.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 1/5/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROSceneRendererOVR.h"

#pragma mark - Setup

VROSceneRendererOVR::VROSceneRendererOVR() {
    _driver = std::make_shared<VRODriverOpenGLAndroid>({});
}

VROSceneRendererOVR::~VROSceneRendererOVR() {

}

#pragma mark - Rendering

void VROSceneRendererOVR::initGL() {
}


void VROSceneRendererOVR::onDrawFrame() {

}

void VROSceneRendererOVR::onTriggerEvent() {

}

void VROSceneRendererOVR::onPause() {

}

void VROSceneRendererOVR::onResume() {

}

#pragma mark - Utility Methods

