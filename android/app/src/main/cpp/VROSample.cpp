//
//  VROSample.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 11/9/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//
#include <VROARShadow.h>
#include "VROSample.h"
#include "VRODriverOpenGLAndroid.h"
#include "VRORendererTestHarness.h"
#include "VRORendererTest.h"
#include "VRONode.h"

VROSample::VROSample() {

}

VROSample::~VROSample() {

}

void VROSample::loadTestHarness(std::shared_ptr<VRORenderer> renderer,
                                std::shared_ptr<VROFrameSynchronizer> frameSynchronizer,
                                std::shared_ptr<VRODriver> driver) {
    _harness = std::make_shared<VRORendererTestHarness>(renderer, frameSynchronizer, driver);
    _harness->loadTest(VRORendererTestType::DiffuseIrradiance);
}

std::shared_ptr<VROSceneController> VROSample::getSceneController() {
    return _harness->getCurrentTest()->getSceneController();
}

std::shared_ptr<VRONode> VROSample::getPointOfView() {
    return _harness->getCurrentTest()->getPointOfView();
}


