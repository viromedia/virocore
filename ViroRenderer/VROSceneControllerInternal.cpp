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
#include "VROReticleSizeListener.h"

static const float kHoverControllerRadiusDegrees = 1;

VROSceneControllerInternal::VROSceneControllerInternal() :
    _scene(std::make_shared<VROScene>()),
    _hoverController(std::make_shared<VROHoverController>(toRadians(kHoverControllerRadiusDegrees), _scene)) {

}

VROSceneControllerInternal::~VROSceneControllerInternal() {
    
}

void VROSceneControllerInternal::attach(std::shared_ptr<VROReticle> reticle,
                                        std::shared_ptr<VROFrameSynchronizer> frameSynchronizer) {
    
    _frameSynchronizer = frameSynchronizer;
    
    if (_reticleSizeListener) {
        _hoverController->removeHoverDistanceListener(_reticleSizeListener);
    }
    _reticleSizeListener = std::make_shared<VROReticleSizeListener>(reticle);
    _hoverController->addHoverDistanceListener(_reticleSizeListener);
    
    frameSynchronizer->addFrameListener(_hoverController);
}

void VROSceneControllerInternal::setHoverEnabled(bool enabled, bool boundsOnly) {
    _hoverTestBoundsOnly = boundsOnly;
    if (enabled) {
        _hoverController->setDelegate(shared_from_this());
    }
    else {
        _hoverController->setDelegate({});
    }
}

void VROSceneControllerInternal::onSceneWillAppear(VRORenderContext &context, VRODriver &driver) {
    
}

void VROSceneControllerInternal::onSceneDidAppear(VRORenderContext &context, VRODriver &driver) {

}

void VROSceneControllerInternal::onSceneWillDisappear(VRORenderContext &context, VRODriver &driver) {
    std::shared_ptr<VROFrameSynchronizer> synchronizer = _frameSynchronizer.lock();
    if (synchronizer) {
        synchronizer->removeFrameListener(_hoverController);
    }
}

void VROSceneControllerInternal::onSceneDidDisappear(VRORenderContext &context, VRODriver &driver) {
    
}
