//
//  VROHoverController.cpp
//  ViroRenderer
//
//  Created by Raj Advani on 2/7/16.
//  Copyright © 2016 Viro Media. All rights reserved.
//

#include "VROHoverController.h"
#include "VRORenderContext.h"
#include "VROLog.h"
#include "VROScene.h"
#include "VRONode.h"
#include "VROHitTestResult.h"

VROHoverController::VROHoverController(float rotationThresholdRadians,
                                       std::shared_ptr<VROScene> scene,
                                       std::function<void(VRONode *const node)> hoverOn,
                                       std::function<void(VRONode *const node)> hoverOff) :
    _scene(scene),
    _hoverOn(hoverOn),
    _hoverOff(hoverOff),
    _rotationThresholdRadians(rotationThresholdRadians),
    _lastCameraForward({ 0, 0, -1}) {
    
}

VROHoverController::~VROHoverController() {
    
}

void VROHoverController::findHoveredNode(VROVector3f ray, std::shared_ptr<VROScene> &scene) {
    std::shared_ptr<VRONode> oldHover = _hoveredNode.lock();

    for (std::shared_ptr<VRONode> &node : scene->getRootNodes()) {
        std::vector<VROHitTestResult> hits = node->hitTest(ray, true);
        if (!hits.empty()) {
            std::shared_ptr<VRONode> newHover = hits.front().getNode();
            
            if (oldHover) {
                if (oldHover != newHover) {
                    _hoverOff(oldHover.get());
                    _hoverOn(newHover.get());
                    
                    _hoveredNode = newHover;
                }
            }
            else {
                _hoverOn(newHover.get());
                _hoveredNode = newHover;
            }
            return;
        }
        else if (oldHover) {
            _hoverOff(oldHover.get());
            _hoveredNode.reset();
        }
    }
}

void VROHoverController::onFrameWillRender(const VRORenderContext &context) {
    std::shared_ptr<VROScene> scene = _scene.lock();
    if (!scene) {
        return;
    }
    
    VROVector3f currentCameraForward = context.getCameraForward();
    VROQuaternion distance = VROQuaternion::rotationFromTo(_lastCameraForward, currentCameraForward);
    
    if (distance.getAngle() > _rotationThresholdRadians) {
        findHoveredNode(currentCameraForward, scene);
        _lastCameraForward = currentCameraForward;
    }
}

void VROHoverController::onFrameDidRender(const VRORenderContext &context) {
    
}