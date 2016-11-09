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

VROSceneControllerInternal::VROSceneControllerInternal(std::shared_ptr<VROReticle> reticle,
                                                       std::shared_ptr<VROFrameSynchronizer> frameSynchronizer) :
    _scene(std::make_shared<VROScene>()),
    _frameSynchronizer(frameSynchronizer) {
    
    std::shared_ptr<VROHoverDistanceListener> reticleSizeListener = std::make_shared<VROReticleSizeListener>(reticle);
        
    _hoverController = std::make_shared<VROHoverController>(toRadians(kHoverControllerRadiusDegrees), _scene);
    _hoverController->addHoverDistanceListener(reticleSizeListener);
}

VROSceneControllerInternal::~VROSceneControllerInternal() {
    
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
    if (!_hoverController) {
        _hoverController = std::make_shared<VROHoverController>(toRadians(kHoverControllerRadiusDegrees),
                                                                _scene);
    }
    
    std::shared_ptr<VROFrameSynchronizer> synchronizer = _frameSynchronizer.lock();
    if (synchronizer) {
        synchronizer->addFrameListener(_hoverController);
    }
}

void VROSceneControllerInternal::onSceneWillDisappear(VRORenderContext &context, VRODriver &driver) {
    std::shared_ptr<VROFrameSynchronizer> synchronizer = _frameSynchronizer.lock();
    if (synchronizer) {
        synchronizer->removeFrameListener(_hoverController);
    }
}

void VROSceneControllerInternal::onSceneDidDisappear(VRORenderContext &context, VRODriver &driver) {
    
}
