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
    _firstHoverEvent(true),
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

/**
 * Perform a hit test on each node within the scene and returns immediately
 * once something has been hit and is hoverable. Returns nil if nothing has been
 * hit - the user would be gazing directly into the background of the scene.
 */
std::shared_ptr<VRONode> VROHoverController::findHoveredNode(VROVector3f ray, std::shared_ptr<VROScene> &scene,
                                         const VRORenderContext &context) {
    VROVector3f cameraPosition = context.getCamera().getPosition();

    std::shared_ptr<VROHoverDelegate> delegate = _delegate.lock();
    bool hitTestBoundsOnly = delegate ? delegate->isHoverTestBoundsOnly() : false;

    std::shared_ptr<VRONode> newHover;
    float minDistance = FLT_MAX;
    float minDistanceHoverable = FLT_MAX;

    for (std::shared_ptr<VRONode> &node : scene->getRootNodes()) {
        std::vector<VROHitTestResult> hits = node->hitTest(ray, context, hitTestBoundsOnly);
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

        if (newHover) {
            break;
        }
    }

    for (std::shared_ptr<VROHoverDistanceListener> &listener : _distanceListeners) {
        listener->onHoverDistanceChanged(minDistance, context);
    }
    return newHover;
}

/**
 * Notify hover events via the VROHoverDelegate with a given VRONode upon which the
 * event applies to. newHover will be nil if the user is gazing into the background
 * of the current scene.
 */
void VROHoverController::notifyDelegatesOnHoveredNode(std::shared_ptr<VRONode> newHover) {
    std::shared_ptr<VROHoverDelegate> delegate = _delegate.lock();
    if (!delegate) {
        return;
    }

    std::shared_ptr<VRONode> oldHover = _hoveredNode.lock();
    if (_firstHoverEvent == true) {
        _firstHoverEvent = false;
        _hoveredNode = newHover;
        delegate->hoverOnNode(newHover);
    } else if (oldHover != newHover) {
        delegate->hoverOffNode(oldHover);
        delegate->hoverOnNode(newHover);
        _hoveredNode = newHover;
    }

    if (!newHover && oldHover) {
        _hoveredNode.reset();
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
        std::shared_ptr<VRONode> hoveredNode = findHoveredNode(currentCameraForward, scene, context);
        notifyDelegatesOnHoveredNode(hoveredNode);
        _lastCameraForward = currentCameraForward;
    }
}

void VROHoverController::onFrameDidRender(const VRORenderContext &context) {
    
}
