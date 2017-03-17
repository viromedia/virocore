//
//  VROInputControllerBase.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROInputControllerBase.h"

static float kSceneBackgroundDistance = 8;
static bool sSceneBackgroundAdd = true;

void VROInputControllerBase::debugMoveReticle() {
    if (sSceneBackgroundAdd) {
        kSceneBackgroundDistance += 0.1;
        if (kSceneBackgroundDistance > 20) {
            sSceneBackgroundAdd = false;
        }
    }
    else {
        kSceneBackgroundDistance -= 0.1;
        if (kSceneBackgroundDistance < 0) {
            sSceneBackgroundAdd = true;
        }
    }

#if VRO_PLATFORM_IOS
    pinfo("Background distance is %f", kSceneBackgroundDistance);
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        debugMoveReticle();
    });
#endif
}

void VROInputControllerBase::setContext(std::shared_ptr<VRORenderContext> context){
    _context = context;
    _controllerPresenter = createPresenter(context);
    registerEventDelegate(_controllerPresenter);

#if VRO_PLATFORM_IOS
    if (kDebugSceneBackgroundDistance) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            debugMoveReticle();
        });
    }
#endif
}

void VROInputControllerBase::onButtonEvent(int source, VROEventDelegate::ClickState clickState){
    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onClick(source, clickState);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitResult == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnClick, _hitResult->getNode());
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
        if (_hitResult->getNode() == _lastClickedNode) {
            for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
                delegate->onClick(source, VROEventDelegate::ClickState::Clicked);
            }
            if (focusedNode != nullptr && _lastClickedNode != nullptr) {
                focusedNode->getEventDelegate()->onClick(source,
                                                         VROEventDelegate::ClickState::Clicked);
            }
        }

        _lastClickedNode = nullptr;
        if (_lastDraggedNode != nullptr) {
            _lastDraggedNode->_draggedNode->setSelectable(true);
        }
        _lastDraggedNode = nullptr;
    } else if (clickState == VROEventDelegate::ClickDown){
        _lastClickedNode = _hitResult->getNode();

        // Identify if object is draggable.
        std::shared_ptr<VRONode> draggableNode
                = getNodeToHandleEvent(VROEventDelegate::EventAction::OnDrag,
                                       _hitResult->getNode());

        if (draggableNode == nullptr){
            return;
        }

        draggableNode->setSelectable(false);

        /*
         * Grab and save a reference to the draggedNode that we will be tracking.
         * Grab and save the distance of the hit result from the controller.
         * Grab and save the hit location from the hit test and original draggedNode position.
         * For each of the above, store them within _lastDraggedNode to be used later
         * within onMove to calculate the new dragged location of the draggedNode
         * in reference to the controller's movement.
         */
        std::shared_ptr<VRODraggedObject> draggedObject = std::make_shared<VRODraggedObject>();
        draggedObject->_draggedDistanceFromController = _hitResult->getLocation().distanceAccurate(_lastKnownPosition);
        draggedObject->_originalHitLocation = _hitResult->getLocation();
        draggedObject->_originalDraggedNodePosition = draggableNode->getPosition();
        draggedObject->_draggedNode = draggableNode;
        _lastDraggedNode  = draggedObject;
    }
}

void VROInputControllerBase::onTouchpadEvent(int source, VROEventDelegate::TouchState touchState,
                                             float posX,
                                             float posY) {
    // Avoid spamming similar TouchDownMove events.
    VROVector3f currentTouchedPosition = VROVector3f(posX, posY, 0);
    if (touchState == VROEventDelegate::TouchState::TouchDownMove &&
        _lastTouchedPosition.isEqual(currentTouchedPosition)) {
        return;
    }
    _lastTouchedPosition = currentTouchedPosition;

    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onTouch(source, touchState, posX, posY);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitResult == nullptr) {
        return;
    }

    std::shared_ptr<VRONode> focusedNode = getNodeToHandleEvent(
            VROEventDelegate::EventAction::OnTouch, _hitResult->getNode());
    if (focusedNode != nullptr) {
        focusedNode->getEventDelegate()->onTouch(source, touchState, posX, posY);
    }
}

void VROInputControllerBase::onMove(int source, VROVector3f position, VROQuaternion rotation) {
    _lastKnownRotation = rotation;
    _lastKnownPosition = position;

    // Trigger orientation delegate callbacks for non-scene elements.
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
        delegate->onGazeHit(source, *_hitResult.get());
        delegate->onMove(source, _lastKnownRotation.toEuler(), _lastKnownPosition);
    }

    if (_hitResult == nullptr){
        return;
    }

    // Trigger orientation delegate callbacks within the scene.
    std::shared_ptr<VRONode> gazableNode
            = getNodeToHandleEvent(VROEventDelegate::EventAction::OnHover, _hitResult->getNode());
    processGazeEvent(source, gazableNode);

    std::shared_ptr<VRONode> movableNode
            = getNodeToHandleEvent(VROEventDelegate::EventAction::OnMove, _hitResult->getNode());
    if (movableNode != nullptr){
        movableNode->getEventDelegate()->onMove(source, _lastKnownRotation.toEuler(), _lastKnownPosition);
    }

    // Update draggable objects if needed
    if (_lastDraggedNode != nullptr){

        // Calculate the new drag location
        VROVector3f newSimulatedHitPosition = _lastKnownForward * _lastDraggedNode->_draggedDistanceFromController;
        VROVector3f draggedOffset = newSimulatedHitPosition - _lastDraggedNode->_originalHitLocation;
        VROVector3f draggedToLocation = _lastDraggedNode->_originalDraggedNodePosition + draggedOffset;
        std::shared_ptr<VRONode> draggedNode = _lastDraggedNode->_draggedNode;
        draggedNode->setPosition(draggedToLocation);

        /*
         * To avoid spamming the JNI / JS bridge, throttle the notification
         * of onDrag delegates to a certain degree of accuracy.
         */
        float distance = draggedToLocation.distance(_lastDraggedNodePosition);
        if (distance < ON_DRAG_DISTANCE_THRESHOLD){
            return;
        }

        // Update last known dragged position and notify delegates
        _lastDraggedNodePosition = draggedToLocation;
        draggedNode->getEventDelegate()->onDrag(source, draggedToLocation);
        for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
            delegate->onDrag(source, draggedToLocation);
        }
    }
}

void VROInputControllerBase::updateHitNode(VROVector3f fromPosition, VROVector3f withDirection){
    if (_scene == nullptr) {
        return;
    }

    // Perform hit test re-calculate forward vectors as needed.
    _hitResult = std::make_shared<VROHitTestResult>(hitTest(withDirection, fromPosition, true));
    _lastKnownForward = withDirection;
}

void VROInputControllerBase::onControllerStatus(int source, VROEventDelegate::ControllerStatus status){
    if (_currentControllerStatus == status){
        return;
    }

    _currentControllerStatus = status;

    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onControllerStatus(source, status);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitResult == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode
            = getNodeToHandleEvent(VROEventDelegate::EventAction::OnControllerStatus, _hitResult->getNode());
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onControllerStatus(source, status);
    }
}

void VROInputControllerBase::onSwipe(int source, VROEventDelegate::SwipeState swipeState) {
    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onSwipe(source, swipeState);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitResult == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode
            = getNodeToHandleEvent(VROEventDelegate::EventAction::OnSwipe, _hitResult->getNode());
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onSwipe(source, swipeState);
    }
}

void VROInputControllerBase::onScroll(int source, float x, float y) {
    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onScroll(source, x, y);
    }

    // Return if we have not focused on any node upon which to trigger events.
    if (_hitResult == nullptr){
        return;
    }

    std::shared_ptr<VRONode> focusedNode
            = getNodeToHandleEvent(VROEventDelegate::EventAction::OnScroll, _hitResult->getNode());
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onScroll(source, x, y);
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

VROHitTestResult VROInputControllerBase::hitTest(VROVector3f ray, VROVector3f hitFromPosition, bool boundsOnly) {
    std::vector<VROHitTestResult> results;
    std::vector<std::shared_ptr<VRONode>> sceneRootNodes = _scene->getRootNodes();

    // Grab all the nodes that were hit
    for (std::shared_ptr<VRONode> node: sceneRootNodes){
        std::vector<VROHitTestResult> nodeResults = node->hitTest(ray, hitFromPosition, boundsOnly);
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

    VROVector3f backgroundPosition =  hitFromPosition + (ray * kSceneBackgroundDistance);

    VROHitTestResult sceneBackgroundHitResult = {_scene->getRootNodes()[0], backgroundPosition , kSceneBackgroundDistance, true};
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