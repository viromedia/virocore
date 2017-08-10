//
//  VROChoreographer.cpp
//  ViroKit
//
//  Created by Raj Advani on 8/9/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROChoreographer.h"
#include "VRORenderPass.h"
#include "VRODriver.h"

VROChoreographer::VROChoreographer() {
    
}

VROChoreographer::~VROChoreographer() {
    
}

void VROChoreographer::render(std::shared_ptr<VROScene> scene, VRORenderContext *context,
                              std::shared_ptr<VRODriver> &driver) {
    
    VRORenderPassInputOutput inputs;
    inputs[kRenderTargetSingleOutput] = driver->getDisplay();
    _baseRenderPass->render(scene, inputs, *context, driver);
}

