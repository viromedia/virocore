//
//  VROInputControllerBase.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROInputControllerBase.h"

void VROInputControllerBase::onButtonEvent(VROEventDelegate::EventSource event,
                                           VROEventDelegate::EventAction action){
    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onButtonEvent(event, action);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitNode == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode = getNodeToHandleEvent(event, _hitNode);
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onButtonEvent(event, action);
    }
}

void VROInputControllerBase::onTouchpadEvent(VROEventDelegate::EventSource event,
                                             VROEventDelegate::EventAction action,
                                             float lastKnownXPos,
                                             float lastKnownYPos){
    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onTouchPadEvent(event, action, lastKnownXPos, lastKnownYPos);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitNode == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode = getNodeToHandleEvent(event, _hitNode);
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onTouchPadEvent(event, action, lastKnownXPos, lastKnownYPos);
    }
}

void VROInputControllerBase::onRotate(VROQuaternion rotation){
     _lastKnownRotation = rotation;
     _lastKnownForward = _lastKnownRotation.getMatrix().multiply(kBaseForward);
     updateControllerOrientation();
}

void VROInputControllerBase::onPosition(VROVector3f position){
     _lastKnownPosition = position;
     updateControllerOrientation();
}

void VROInputControllerBase::updateControllerOrientation(){
    // Return for now if scene is not ready - it is required for the hit test.
    if (_scene == nullptr){
        return;
    }

    // Perform hit test re-calculate forward vectors as needed.
    VROHitTestResult hitResult = hitTest(_lastKnownForward, _lastKnownPosition, true);
    VROVector3f rotationEuler =_lastKnownRotation.toEuler();
    _hitNode = hitResult.getNode();

    // Trigger orientation delegate callbacks within the scene.
    std::shared_ptr<VRONode> gazableNode
            = getNodeToHandleEvent(VROEventDelegate::EventSource::CONTROLLER_GAZE, hitResult.getNode());
    processGazeEvent(gazableNode);

    std::shared_ptr<VRONode> rotatableNode
            = getNodeToHandleEvent(VROEventDelegate::EventSource::CONTROLLER_ROTATE, hitResult.getNode());
    if (rotatableNode != nullptr){
        rotatableNode->getEventDelegate()->onRotate(rotationEuler);
    }

    std::shared_ptr<VRONode> positionableNode
            = getNodeToHandleEvent(VROEventDelegate::EventSource::CONTROLLER_MOVEMENT, hitResult.getNode());
    if (positionableNode != nullptr){
        positionableNode->getEventDelegate()->onPosition(_lastKnownPosition);
    }

    // Trigger orientation delegate callbacks for non-scene elements.
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onGazeHit(hitResult.getDistance(), hitResult.getLocation());
        delegate->onRotate(rotationEuler);
        delegate->onPosition(_lastKnownPosition);
    }
}

void VROInputControllerBase::onControllerStatus(VROEventDelegate::ControllerStatus status){
    if (_currentControllerStatus == status){
        return;
    }

    _currentControllerStatus = status;

    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onControllerStatus(status);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitNode == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode
            = getNodeToHandleEvent(VROEventDelegate::EventSource ::CONTROLLER_STATUS, _hitNode);
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onControllerStatus(status);
    }
}

void VROInputControllerBase::processGazeEvent(std::shared_ptr<VRONode> newNode) {
    if (_hoveredNode == newNode){
        return;
    }

    if (newNode) {
        newNode->getEventDelegate()->onGaze(true);
    }

    if (_hoveredNode){
        _hoveredNode->getEventDelegate()->onGaze(false);
    }
    _hoveredNode = newNode;
}

VROHitTestResult VROInputControllerBase::hitTest(VROVector3f vector, VROVector3f hitFromPosition, bool boundsOnly) {
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

   VROVector3f backgroundPosition =  hitFromPosition + (vector * SCENE_BACKGROUND_DIST);
   VROHitTestResult sceneBackgroundHitResult = {_scene->getRootNodes()[0], backgroundPosition , SCENE_BACKGROUND_DIST};
   return sceneBackgroundHitResult;
}

std::shared_ptr<VRONode> VROInputControllerBase::getNodeToHandleEvent(VROEventDelegate::EventSource type,
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
