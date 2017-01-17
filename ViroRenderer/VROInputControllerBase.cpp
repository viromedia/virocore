//
//  VROInputControllerBase.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROInputControllerBase.h"

void VROInputControllerBase::onButtonEvent(int source, VROEventDelegate::ClickState clickState){
    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onClick(source, clickState);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitNode == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnClick, _hitNode);
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onClick(source, clickState);
    }

    /*
     * If we have completed a ClickUp and ClickDown event sequentially for a
     * given Node, trigger an onClicked event.
     *
     * NOTE: This only tracks the last node that was CLICKED_DOWN irregardless of source;
     * it does not consider the corner case where DOWN / UP events may be performed from
     * different sources.
     */
    if (clickState == VROEventDelegate::ClickUp) {
        if (focusedNode != nullptr && _lastClickedNode != nullptr && _hitNode == _lastClickedNode) {
            focusedNode->getEventDelegate()->onClick(source, VROEventDelegate::ClickState::Clicked);
        }
        _lastClickedNode = nullptr;
    } else if (clickState == VROEventDelegate::ClickDown){
        _lastClickedNode = _hitNode;
    }
}

void VROInputControllerBase::onTouchpadEvent(int source, VROEventDelegate::TouchState touchState,
                                             float posX,
                                             float posY){

    // Avoid spamming similar TouchDownMove events.
    VROVector3f currentTouchedPosition = VROVector3f(posX, posY, 0);
    if (touchState == VROEventDelegate::TouchState::TouchDownMove &&
            _lastTouchedPosition.isEqual(currentTouchedPosition)){
        return;
    }
    _lastTouchedPosition = currentTouchedPosition;

    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onTouch(source, touchState, posX, posY);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitNode == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnTouch, _hitNode);
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onTouch(source, touchState, posX, posY);
    }
}

void VROInputControllerBase::onRotate(int source, const VROQuaternion rotation){
    if (_lastKnownRotation == rotation){
        return;
    }

    _lastKnownRotation = rotation;
    _lastKnownForward = _lastKnownRotation.getMatrix().multiply(kBaseForward);
    updateControllerOrientation(source);
}

void VROInputControllerBase::onPosition(int source, VROVector3f position){
     _lastKnownPosition = position;
     updateControllerOrientation(source);
}

void VROInputControllerBase::updateControllerOrientation(int source){
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
            = getNodeToHandleEvent(VROEventDelegate::EventAction::OnHover, hitResult.getNode());
    processGazeEvent(source, gazableNode);

    std::shared_ptr<VRONode> movableNode
            = getNodeToHandleEvent(VROEventDelegate::EventAction::OnMove, hitResult.getNode());
    if (movableNode != nullptr){
        movableNode->getEventDelegate()->onMove(source, rotationEuler, _lastKnownPosition);
    }

    // Trigger orientation delegate callbacks for non-scene elements.
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onGazeHit(source, hitResult.getDistance(), hitResult.getLocation());
        delegate->onMove(source, rotationEuler, _lastKnownPosition);
    }
}

void VROInputControllerBase::onControllerStatus(int source, VROEventDelegate::ControllerStatus status){
    if (_currentControllerStatus == status){
        return;
    }

    _currentControllerStatus = status;

    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onControllerStatus(source, status);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitNode == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode
            = getNodeToHandleEvent(VROEventDelegate::EventAction::OnControllerStatus, _hitNode);
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onControllerStatus(source, status);
    }
}

void VROInputControllerBase::processGazeEvent(int source, std::shared_ptr<VRONode> newNode) {
    if (_lastHoveredNode == newNode){
        return;
    }

    if (newNode) {
        newNode->getEventDelegate()->onHover(source, true);
    }

    if (_lastHoveredNode){
        _lastHoveredNode->getEventDelegate()->onHover(source, false);
    }
    _lastHoveredNode = newNode;
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

std::shared_ptr<VRONode> VROInputControllerBase::getNodeToHandleEvent(VROEventDelegate::EventAction action,
                                                               std::shared_ptr<VRONode> node){
    // Base condition, we are asking for the scene's root node's parent, return.
    if (node == nullptr){
        return nullptr;
    }

    std::shared_ptr<VROEventDelegate> delegate = node->getEventDelegate();
    if (delegate != nullptr && delegate->isEventEnabled(action)){
        return node;
    } else {
        return getNodeToHandleEvent(action, node->getParentNode());
    }
}
