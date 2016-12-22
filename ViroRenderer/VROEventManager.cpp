//
//  VROEventManager.cpp
//  ViroRenderer
//
//  Copyright © 2015 Viro Media. All rights reserved.
//

#include <float.h>
#include "VROEventManager.h"
#include "VRONode.h"
#include "VROHitTestResult.h"
#include <android/log.h>

void VROEventManager::onHeadGearTap() {
    // Return for now if scene is not ready - it is required for the hit test.
    if (_scene == nullptr){
        return;
    }

    VROCamera camera = _context->getCamera();
    VROHitTestResult hitResult = hitTest(camera.getForward(), camera.getPosition(), true);
    std::shared_ptr<VRONode> tappableNode
            = getNodeToHandleEvent(VROEventDelegate::EventType::ON_TAP, hitResult.getNode());

    if (tappableNode != nullptr){
        tappableNode->getEventDelegate()->onTapped();
    }
}

void VROEventManager::onHeadGearGaze() {
    // Return for now if scene is not ready - it is required for the hit test.
    if (_scene == nullptr){
        return;
    }

    VROCamera camera = _context->getCamera();
    VROHitTestResult hitResult = hitTest(camera.getForward(), camera.getPosition(), true);

    // Notify all the nodes within the scene graph about new event
    std::shared_ptr<VRONode> gazableNode
           = getNodeToHandleEvent(VROEventDelegate::EventType::ON_GAZE, hitResult.getNode());
    processGazeEvent(gazableNode);

    // Notify non-scene delegates about the gazed distance event
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onGazeHitDistance(hitResult.getDistance());
    }
}

/**
 * Returns the closest node that was hit.
 */
VROHitTestResult VROEventManager::hitTest(VROVector3f vector, VROVector3f hitFromPosition, bool boundsOnly) {
    std::vector<VROHitTestResult> results;
    std::vector<std::shared_ptr<VRONode>> sceneRootNodes = _scene->getRootNodes();

    // Grab all the nodes that were hit
    for (std::shared_ptr<VRONode> node: sceneRootNodes){
        std::vector<VROHitTestResult> nodeResults = node->hitTest(vector, hitFromPosition, boundsOnly);
        results.insert(results.end(), nodeResults.begin(), nodeResults.end());
    }

    // Sort and get the closest node
    std::sort(results.begin(), results.end(), [](VROHitTestResult a, VROHitTestResult b) {
        return a.getDistance() < b.getDistance();
    });

    // Return the closest hit element, if any.
    if (results.size() > 0) {
        return results[0];
    }

    // Else, if nothing's hit, return the scene background node
    VROVector3f infinitePosition = VROVector3f(FLT_MAX, FLT_MAX, FLT_MAX);
    VROHitTestResult sceneBackgroundHitResult = {_scene->getRootNodes()[0], infinitePosition , SCENE_BACKGROUND_DIST};
    return sceneBackgroundHitResult;
}

void VROEventManager::processGazeEvent(std::shared_ptr<VRONode> newNode) {
    if (_gazedNode == newNode){
        return;
    }

    if (newNode) {
        newNode->getEventDelegate()->onGaze(true);
    }

    if (_gazedNode){
        _gazedNode->getEventDelegate()->onGaze(false);
    }
    _gazedNode = newNode;
}

/**
 * Returns the first node that is able to handle the event by bubbling it up.
 * If nothing is able to handle the event, nullptr is returned.
 */
std::shared_ptr<VRONode> VROEventManager::getNodeToHandleEvent(VROEventDelegate::EventType type,
                                                               std::shared_ptr<VRONode> node){
    // Base condition, we are asking for the scene's root node's parent, return.
    if (node == nullptr){
        return nullptr;
    }

    std::shared_ptr<VROEventDelegate> delegate = node->getEventDelegate();
    if (delegate != nullptr && delegate->isEventEnabled(type)){
        return node;
    } else {
        return getNodeToHandleEvent(type, node->getParentNode());
    }
}
