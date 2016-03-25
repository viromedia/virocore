//
//  VROSceneController.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 3/25/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROSceneControllerInternal.h"
#include "VRORenderContext.h"
#include "VROHoverController.h"
#include "VROScene.h"
#include "VROFrameListener.h"
#include "VROLog.h"

static const float kHoverControllerRadiusDegrees = 1;

VROSceneControllerInternal::VROSceneControllerInternal() {
    _scene = std::make_shared<VROScene>();
}

VROSceneControllerInternal::~VROSceneControllerInternal() {
    
}

void VROSceneControllerInternal::setHoverDelegate(std::shared_ptr<VROHoverDelegate> delegate) {
    if (!_hoverController) {
        _hoverController = std::make_shared<VROHoverController>(toRadians(kHoverControllerRadiusDegrees),
                                                                _scene);
    }
    
    _hoverController->setDelegate(delegate);
}

void VROSceneControllerInternal::onSceneWillAppear(VRORenderContext &context) {
    
}

void VROSceneControllerInternal::onSceneDidAppear(VRORenderContext &context) {
    if (!_hoverController) {
        _hoverController = std::make_shared<VROHoverController>(toRadians(kHoverControllerRadiusDegrees),
                                                                _scene);
    }
    
    context.addFrameListener(_hoverController);
}

void VROSceneControllerInternal::onSceneWillDisappear(VRORenderContext &context) {
    context.removeFrameListener(_hoverController);
}

void VROSceneControllerInternal::onSceneDidDisappear(VRORenderContext &context) {
    
}