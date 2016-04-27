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
#include "VROFrameSynchronizer.h"
#include "VROLog.h"

static const float kHoverControllerRadiusDegrees = 1;

VROSceneControllerInternal::VROSceneControllerInternal(std::shared_ptr<VROHoverDistanceListener> reticleSizeListener) {
    _scene = std::make_shared<VROScene>();
    
    _hoverController = std::make_shared<VROHoverController>(toRadians(kHoverControllerRadiusDegrees), _scene);
    _hoverController->addHoverDistanceListener(reticleSizeListener);
}

VROSceneControllerInternal::~VROSceneControllerInternal() {
    
}

void VROSceneControllerInternal::setHoverDelegate(std::shared_ptr<VROHoverDelegate> delegate) {
    if (!_hoverController) {
        
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
    
    std::shared_ptr<VROFrameSynchronizer> synchronizer = _frameSynchronizer.lock();
    if (synchronizer) {
        synchronizer->addFrameListener(_hoverController);
    }
}

void VROSceneControllerInternal::onSceneWillDisappear(VRORenderContext &context) {
    std::shared_ptr<VROFrameSynchronizer> synchronizer = _frameSynchronizer.lock();
    if (synchronizer) {
        synchronizer->removeFrameListener(_hoverController);
    }
}

void VROSceneControllerInternal::onSceneDidDisappear(VRORenderContext &context) {
    
}