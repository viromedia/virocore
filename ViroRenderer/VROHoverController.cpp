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
#include "VROHoverDelegate.h"
#include "VROHoverDistanceListener.h"

VROHoverController::VROHoverController(float rotationThresholdRadians,
                                       std::shared_ptr<VROScene> scene) :
    _scene(scene),
    _rotationThresholdRadians(rotationThresholdRadians),
    _lastCameraForward({ 0, 0, 1}) {
    
}

VROHoverController::~VROHoverController() {
    
}

void VROHoverController::setDelegate(std::shared_ptr<VROHoverDelegate> delegate) {
    _delegate = delegate;
}

void VROHoverController::addHoverDistanceListener(std::shared_ptr<VROHoverDistanceListener> listener) {
    _distanceListeners.push_back(listener);
}

void VROHoverController::removeHoverDistanceListener(std::shared_ptr<VROHoverDistanceListener> listener) {
    _distanceListeners.erase(
                             std::remove_if(_distanceListeners.begin(), _distanceListeners.end(),
                                            [listener](std::shared_ptr<VROHoverDistanceListener> candidate) {
                                                return candidate == listener;
                                            }), _distanceListeners.end());
}

void VROHoverController::findHoveredNode(VROVector3f ray, std::shared_ptr<VROScene> &scene,
                                         const VRORenderContext &context) {
    
    VROVector3f cameraPosition = context.getCamera().getPosition();
    
    std::shared_ptr<VRONode> oldHover = _hoveredNode.lock();
    std::shared_ptr<VROHoverDelegate> delegate = _delegate.lock();
    
    bool hitTestBoundsOnly = delegate ? delegate->isHitTestBoundsOnly() : false;

    for (std::shared_ptr<VRONode> &node : scene->getRootNodes()) {
        std::vector<VROHitTestResult> hits = node->hitTest(ray, context, hitTestBoundsOnly);
        
        float minDistance = FLT_MAX;
        float minDistanceHoverable = FLT_MAX;
        
        std::shared_ptr<VRONode> newHover;
        
        for (VROHitTestResult &hit : hits) {
            float distance = hit.getLocation().distance(cameraPosition);
            if (distance < minDistance) {
                minDistance = distance;
            }
            
            if (delegate && delegate->isHoverable(hit.getNode())) {
                if (distance < minDistanceHoverable) {
                    minDistanceHoverable = distance;
                    newHover = hit.getNode();
                }
            }
        }
        
        for (std::shared_ptr<VROHoverDistanceListener> &listener : _distanceListeners) {
            listener->onHoverDistanceChanged(minDistance);
        }
        
        if (delegate) {
            if (newHover) {
                if (oldHover) {
                    if (oldHover != newHover) {
                        delegate->hoverOff(oldHover);
                        delegate->hoverOn(newHover);
                        
                        _hoveredNode = newHover;
                    }
                }
                else {
                    delegate->hoverOn(newHover);
                    _hoveredNode = newHover;
                }
                return;
            }
            else if (oldHover) {
                delegate->hoverOff(oldHover);
                _hoveredNode.reset();
            }
        }
    }
}

void VROHoverController::onFrameWillRender(const VRORenderContext &context) {
    std::shared_ptr<VROScene> scene = _scene.lock();
    if (!scene) {
        return;
    }
    
    VROVector3f currentCameraForward = context.getCamera().getForward();
    VROQuaternion distance = VROQuaternion::rotationFromTo(_lastCameraForward, currentCameraForward);
    
    if (distance.getAngle() > _rotationThresholdRadians) {
        findHoveredNode(currentCameraForward, scene, context);
        _lastCameraForward = currentCameraForward;
    }
}

void VROHoverController::onFrameDidRender(const VRORenderContext &context) {
    
}