//
//  VROInputControllerBase.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROInputControllerBase.h"
#include "VROTime.h"
#include "VROPortal.h"

static bool sSceneBackgroundAdd = true;

VROInputControllerBase::VROInputControllerBase(std::shared_ptr<VRODriver> driver) :
    _driver(driver) {
    _lastKnownPosition = VROVector3f(0,0,0);
    _lastDraggedNodePosition = VROVector3f(0,0,0);
    _lastClickedNode = nullptr;
    _lastHoveredNode = nullptr;
    _lastDraggedNode = nullptr;
    _currentPinchedNode = nullptr;
    _currentRotateNode = nullptr;
    _scene = nullptr;
    _currentControllerStatus = VROEventDelegate::ControllerStatus::Unknown;
    
#if VRO_PLATFORM_IOS
    if (kDebugSceneBackgroundDistance) {
        dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(0.1 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
            debugMoveReticle();
        });
    }
#endif
}

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

void VROInputControllerBase::setView(VROMatrix4f view) {
    _view = view;
}

void VROInputControllerBase::setProjection(VROMatrix4f projection) {
    _projection = projection;
}

void VROInputControllerBase::onButtonEvent(int source, VROEventDelegate::ClickState clickState) {
    // Return if we have not focused on any node upon which to trigger events.
    if (_hitResult == nullptr) {
        return;
    }

    VROVector3f hitLoc = _hitResult->getLocation();
    std::vector<float> pos = {hitLoc.x, hitLoc.y, hitLoc.z};
    if (_hitResult->isBackgroundHit()) {
        pos.clear();
    }

    // Notify internal delegates
    std::shared_ptr<VRONode> focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnClick, _hitResult->getNode());
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onClick(source, focusedNode, clickState, pos);
    }
    if (focusedNode != nullptr) {
        focusedNode->getEventDelegate()->onClick(source, focusedNode, clickState, pos);
    }

    /*
     If we have completed a ClickUp and ClickDown event sequentially for a
     given Node, trigger an onClicked event.
     
     NOTE: This only tracks the last node that was CLICKED_DOWN regardless of source;
     it does not consider the corner case where DOWN / UP events may be performed from
     different sources.
     */
    if (clickState == VROEventDelegate::ClickUp) {
        if (_hitResult->getNode() == _lastClickedNode) {
            for (std::shared_ptr<VROEventDelegate> delegate : _delegates){
                delegate->onClick(source, focusedNode, VROEventDelegate::ClickState::Clicked, pos);
            }
            if (focusedNode != nullptr && focusedNode->getEventDelegate() && _lastClickedNode != nullptr) {
                focusedNode->getEventDelegate()->onClick(source, focusedNode,
                                                         VROEventDelegate::ClickState::Clicked,
                                                         pos);
            }
        }
        _lastClickedNode = nullptr;
        if (_lastDraggedNode != nullptr) {
            _lastDraggedNode->_dragState = VROEventDelegate::DragState::End;
            _lastDraggedNode->_draggedNode->setIsBeingDragged(false);
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

        /*
         Grab and save a reference to the draggedNode that we will be tracking.
         Grab and save the distance of the hit result from the controller.
         Grab and save the hit location from the hit test and original draggedNode position.
         For each of the above, store them within _lastDraggedNode to be used later
         within onMove to calculate the new dragged location of the draggedNode
         in reference to the controller's movement.
         */
        std::shared_ptr<VRODraggedObject> draggedObject = std::make_shared<VRODraggedObject>();
        draggedObject->_originalDraggedNodePosition = draggableNode->getWorldPosition();
        draggedObject->_originalDraggedNodeRotation = draggableNode->getWorldRotation();
        draggedObject->_draggedNode = draggableNode;
        _lastDraggedNode = draggedObject;
        _lastDraggedNode->_dragState = VROEventDelegate::DragState::Start;
        draggableNode->setIsBeingDragged(true);
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

    std::shared_ptr<VRONode> focusedNode;
    if (_hitResult) {
        focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnTouch, _hitResult->getNode());
    }
    
    // Notify internal delegates
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onTouch(source, focusedNode, touchState, posX, posY);
    }
    if (focusedNode != nullptr) {
        focusedNode->getEventDelegate()->onTouch(source, focusedNode, touchState, posX, posY);
    }
}

void VROInputControllerBase::onMove(int source, VROVector3f position, VROQuaternion rotation, VROVector3f forward) {
    _lastKnownRotation = rotation;
    _lastKnownPosition = position;
    _lastKnownForward = forward;
    if (_hitResult == nullptr) {
        return;
    }

    // Trigger orientation delegate callbacks within the scene.
    processOnFuseEvent(source, _hitResult->getNode());

    std::shared_ptr<VRONode> movableNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnMove,
                                                                _hitResult->getNode());
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onMove(source, movableNode, _lastKnownRotation.toEuler(), _lastKnownPosition, _lastKnownForward);
    }
    if (movableNode != nullptr) {
        movableNode->getEventDelegate()->onMove(source, movableNode, _lastKnownRotation.toEuler(),
                                                _lastKnownPosition, _lastKnownForward);
    }
    
    // Update draggable objects if needed unless we have a pinch motion.
    if (_lastDraggedNode != nullptr && ((_currentPinchedNode == nullptr) && (_currentRotateNode == nullptr))) {
        processDragging(source);
    }
}

void VROInputControllerBase::processDragging(int source) {
    std::shared_ptr<VRONode> draggedNode = _lastDraggedNode->_draggedNode;

    // Calculate starting pre-drag properties if needed (hit locations, offsets, etc).
    if (_lastDraggedNode->_dragState == VROEventDelegate::DragState::Start) {
        if (draggedNode->getDragType() == VRODragType::FixedDistanceOrigin) {
            _lastDraggedNode->_draggedDistanceFromController = draggedNode->getWorldPosition().distanceAccurate(_lastKnownPosition);
            _lastDraggedNode->_originalHitLocation = draggedNode->getWorldPosition();
        } else if (draggedNode->getDragType() == VRODragType::FixedToPlane) {
            _lastDraggedNode->_originalHitLocation = getPlaneIntersect(draggedNode);

            // Snap object onto the plane if need be.
            VROVector3f p = draggedNode->getDragPlanePoint();
            VROVector3f n = draggedNode->getDragPlaneNormal();
            VROVector3f c = draggedNode->getWorldPosition();
            float isOnPlane = (n.x * (c.x - p.x)) + (n.y * (c.y - p.y)) + (n.z * (c.z -p.z));
            isOnPlane = floorf(isOnPlane * 100) / 100;
            if (isOnPlane != 0.0){
                _lastDraggedNode->_originalDraggedNodePosition = _lastDraggedNode->_originalHitLocation;
            }
        } else {
            _lastDraggedNode->_draggedDistanceFromController = _hitResult->getLocation().distanceAccurate(_lastKnownPosition);
            _lastDraggedNode->_originalHitLocation = _hitResult->getLocation();
        }

        // Grab the forwardOffset (delta from the controller's forward in reference to the user).
        _lastDraggedNode->_forwardOffset = getDragForwardOffset();
        _lastDraggedNode->_dragState = VROEventDelegate::DragState::Dragging;
    }

    // Only calculate drag-to node positions for nodes in dragging states.
    if (_lastDraggedNode->_dragState != VROEventDelegate::DragState::Dragging) {
        return;
    }

    /*
     * Calculate the new drag-to world position based on the DragType for this node.
     */
    VROVector3f draggedToPosition;
    switch(draggedNode->getDragType()) {
        case VRODragType::FixedToPlane:
            draggedToPosition = getDragPositionFixedToPlane();
            break;
        case VRODragType::FixedToWorld: // this is only supported in AR, so default to FixedDistance here
        case VRODragType::FixedDistance:
        case VRODragType::FixedDistanceOrigin:
            draggedToPosition = getDragPositionFixedDistance();
            break;
    }

    draggedNode->setWorldTransform(draggedToPosition, _lastDraggedNode->_originalDraggedNodeRotation);

    /*
     To avoid spamming the JNI / JS bridge, throttle the notification
     of onDrag delegates to a certain degree of accuracy.
     */
    float distance = draggedToPosition.distance(_lastDraggedNodePosition);
    if (distance < ON_DRAG_DISTANCE_THRESHOLD) {
        return;
    }

    // Update last known dragged position and notify delegates
    _lastDraggedNodePosition = draggedToPosition;
    if (draggedNode != nullptr && draggedNode->getEventDelegate()) {
        draggedNode->getEventDelegate()->onDrag(source, draggedNode, draggedToPosition);
    }
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onDrag(source, draggedNode, draggedToPosition);
    }
}

VROVector3f VROInputControllerBase::getDragPositionFixedDistance() {
    // This is the forward plus the offset from the camera to the controller
    VROVector3f adjustedForward = _lastKnownForward + _lastDraggedNode->_forwardOffset;

    // camera position + adjustedForward scaled by the distanceFromController (to maintain fixed distance)
    VROVector3f dragPositionWorld = _lastKnownPosition + (adjustedForward * _lastDraggedNode->_draggedDistanceFromController);

    // The offset is the new drag location minus the original HitTest location
    VROVector3f draggedOffset = dragPositionWorld - _lastDraggedNode->_originalHitLocation;

    // Finally, the position returned is the "starting" position of the dragged object + the offset.
    return _lastDraggedNode->_originalDraggedNodePosition + draggedOffset;
}

VROVector3f VROInputControllerBase::getPlaneIntersect(std::shared_ptr<VRONode> node) {
    // get the information from the node (set by the dev)
    VROVector3f planePoint = node->getDragPlanePoint();
    VROVector3f planeNormal = node->getDragPlaneNormal();
    float maxDistance = node->getDragMaxDistance();

    // if the plane info hasn't been set, then just return the current position.
    if (planeNormal.isZero()) { // we don't check if planePoint is zero because that's a "valid" point
        return node->getPosition();
    }

    // Find the intersection between the plane and the controller forward
    VROVector3f intersectionPoint;
    bool success = _lastKnownForward.rayIntersectPlane(planePoint, planeNormal,
                                                       _lastKnownPosition, &intersectionPoint);

    // if there wasn't an intersection point OR the intersectionPoint was too far from the controller's
    // position, then we want to compute the plane position at maxDistance from the controller's
    // position along the controller's forward. (this is the circle you get b/t intersection of a
    // sphere and a plane).
    if (!success || intersectionPoint.distance(_lastKnownPosition) > maxDistance) {

        // first, project the controller's position onto the plane
        VROVector3f controllerProj;
        success = _lastKnownPosition.projectOnPlane(planePoint, planeNormal, &controllerProj);
        if (!success) {
            return node->getPosition();
        }

        // second, project the controller's position + forward onto the plane
        VROVector3f forwardProj;
        success = _lastKnownPosition.add(_lastKnownForward).projectOnPlane(planePoint, planeNormal, &forwardProj);
        if (!success) {
            return node->getPosition();
        }

        // find the length of the 3rd side of the right handed triangle formed by the controller's
        // position, it's projected point, and the position on the plane "maxDistance" from the controller
        float length = sqrtf(powf(maxDistance, 2) - (powf(_lastKnownPosition.distance(controllerProj), 2)));

        // finally, calculate the intersection point b/t the plane and sphere along the user's forward
        intersectionPoint = controllerProj.add(forwardProj.subtract(controllerProj).normalize().scale(length));
    }

    return intersectionPoint;
}

VROVector3f VROInputControllerBase::getDragPositionFixedToPlane() {
    // The offset is the intersectionPoint minus the original HitTest location
    std::shared_ptr<VRONode> node = _lastDraggedNode->_draggedNode;
    VROVector3f draggedOffset = getPlaneIntersect(node) - _lastDraggedNode->_originalHitLocation;

    // Finally, the position returned is the "starting" position of the dragged object + the offset.
    // This positions the dragged node relative to its parent.
    return _lastDraggedNode->_originalDraggedNodePosition + draggedOffset;
}

void VROInputControllerBase::onPinch(int source, float scaleFactor, VROEventDelegate::PinchState pinchState) {
    if(pinchState == VROEventDelegate::PinchState::PinchStart) {
        if(_hitResult == nullptr) {
            return;
        }
        _lastPinchScale = scaleFactor;
        _currentPinchedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnPinch, _hitResult->getNode());
    }
    
    if(_currentPinchedNode && pinchState == VROEventDelegate::PinchState::PinchMove) {
        if(fabs(scaleFactor - _lastPinchScale) < ON_PINCH_SCALE_THRESHOLD) {
            return;
        }
    }

    if(_currentPinchedNode && _currentPinchedNode->getEventDelegate()) {
        _currentPinchedNode->getEventDelegate()->onPinch(source, _currentPinchedNode, scaleFactor, pinchState);
        if(pinchState == VROEventDelegate::PinchState::PinchEnd) {
            _currentPinchedNode = nullptr;
        }
    }
}

void VROInputControllerBase::onRotate(int source, float rotationRadians, VROEventDelegate::RotateState rotateState) {
    if(rotateState == VROEventDelegate::RotateState::RotateStart) {
        if(_hitResult == nullptr) {
            return;
        }
        _lastRotation = rotationRadians;
        _currentRotateNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnRotate, _hitResult->getNode());
    }
    
    if(_currentRotateNode && rotateState == VROEventDelegate::RotateState::RotateMove) {
        if(fabs(rotationRadians - _lastRotation) < ON_ROTATE_THRESHOLD) {
            return;
        }
    }
    
    if(_currentRotateNode && _currentRotateNode->getEventDelegate()) {
        _currentRotateNode->getEventDelegate()->onRotate(source, _currentRotateNode, rotationRadians, rotateState);
        if(rotateState == VROEventDelegate::RotateState::RotateEnd) {
            _currentRotateNode = nullptr;
        }
    }
}

void VROInputControllerBase::updateHitNode(const VROCamera &camera, VROVector3f origin, VROVector3f ray) {
    if (_scene == nullptr || _lastDraggedNode != nullptr) {
        return;
    }

    // Perform hit test re-calculate forward vectors as needed.
    _hitResult = std::make_shared<VROHitTestResult>(hitTest(camera, origin, ray, true));
}

void VROInputControllerBase::onControllerStatus(int source, VROEventDelegate::ControllerStatus status){
    if (_currentControllerStatus == status){
        return;
    }

    _currentControllerStatus = status;

    std::shared_ptr<VRONode> focusedNode;
    if (_hitResult) {
        focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnControllerStatus, _hitResult->getNode());
    }

    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onControllerStatus(source, status);
    }
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onControllerStatus(source, status);
    }
}

void VROInputControllerBase::onSwipe(int source, VROEventDelegate::SwipeState swipeState) {
    std::shared_ptr<VRONode> focusedNode;
    if (_hitResult) {
        focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnSwipe, _hitResult->getNode());
    }

    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onSwipe(source, focusedNode, swipeState);
    }
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onSwipe(source, focusedNode, swipeState);
    }
}

void VROInputControllerBase::onScroll(int source, float x, float y) {
    std::shared_ptr<VRONode> focusedNode;
    if (_hitResult) {
        focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnScroll, _hitResult->getNode());
    }

    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onScroll(source, focusedNode, x, y);
    }
    if (focusedNode != nullptr){
        focusedNode->getEventDelegate()->onScroll(source, focusedNode, x, y);
    }
}

void VROInputControllerBase::processGazeEvent(int source) {
    std::shared_ptr<VRONode> newNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnHover,
                                                                _hitResult->getNode());
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onGazeHit(source, newNode, *_hitResult.get());
    }

    if (_lastHoveredNode == newNode) {
        return;
    }

    VROVector3f hitLoc = _hitResult->getLocation();
    std::vector<float> pos = {hitLoc.x, hitLoc.y, hitLoc.z};
    if (_hitResult->isBackgroundHit()) {
        pos.clear();
    }

    if (newNode && newNode->getEventDelegate()) {
        std::shared_ptr<VROEventDelegate> delegate = newNode->getEventDelegate();
        if (delegate) {
            delegate->onHover(source, newNode, true, pos);
        }
    }

    if (_lastHoveredNode && _lastHoveredNode->getEventDelegate()) {
        std::shared_ptr<VROEventDelegate> delegate = _lastHoveredNode->getEventDelegate();
        if (delegate) {
            delegate->onHover(source, _lastHoveredNode, false, pos);
        }
    }

    _lastHoveredNode = newNode;
}

void VROInputControllerBase::processOnFuseEvent(int source, std::shared_ptr<VRONode> newNode) {
    std::shared_ptr<VRONode> focusedNode = getNodeToHandleEvent(VROEventDelegate::EventAction::OnFuse, newNode);
    if (_currentFusedNode != focusedNode){
        notifyOnFuseEvent(source, kOnFuseReset);
        _fuseTriggerAtMillis = kOnFuseReset;
        _haveNotifiedOnFuseTriggered = false;
        _currentFusedNode = focusedNode;
    }

    // Do nothing if no onFuse node is found
    if (!focusedNode || !_currentFusedNode->getEventDelegate()){
        return;
    }

    if (_fuseTriggerAtMillis == kOnFuseReset){
        _fuseTriggerAtMillis = VROTimeCurrentMillis() + _currentFusedNode->getEventDelegate()->getTimeToFuse();
    }

    // Compare the fuse time with the current time to get the timeToFuseRatio and notify delegates.
    // When the timeToFuseRatio counts down to 0, it is an indication that the node has been "onFused".
    if (!_haveNotifiedOnFuseTriggered){
        float delta = _fuseTriggerAtMillis - VROTimeCurrentMillis();
        float timeToFuseRatio = delta / _currentFusedNode->getEventDelegate()->getTimeToFuse();

        if (timeToFuseRatio <= 0.0f){
            timeToFuseRatio = 0.0f;
            _haveNotifiedOnFuseTriggered = true;
        }

        notifyOnFuseEvent(source, timeToFuseRatio);
    }
}

void VROInputControllerBase::notifyCameraTransform(const VROCamera &camera) {
    if (_scene) {
        std::shared_ptr<VROEventDelegate> delegate = _scene->getRootNode()->getEventDelegate();

        if (delegate && delegate->isEventEnabled(VROEventDelegate::EventAction::OnCameraTransformUpdate)) {
            delegate->onCameraTransformUpdate(camera.getPosition(), camera.getRotation().toEuler(),
                                              camera.getForward(), camera.getUp());
        }
    }
}

void VROInputControllerBase::notifyOnFuseEvent(int source, float timeToFuseRatio) {
    for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
        delegate->onFuse(source, _currentFusedNode, timeToFuseRatio);
    }

    if (_currentFusedNode && _currentFusedNode->getEventDelegate()){
        _currentFusedNode->getEventDelegate()->onFuse(source, _currentFusedNode, timeToFuseRatio);
    }
}

VROHitTestResult VROInputControllerBase::hitTest(const VROCamera &camera, VROVector3f origin, VROVector3f ray, bool boundsOnly) {
    std::vector<VROHitTestResult> results;
    std::shared_ptr<VROPortal> sceneRootNode = _scene->getRootNode();

    // Grab all the nodes that were hit
    std::vector<VROHitTestResult> nodeResults = sceneRootNode->hitTest(camera, origin, ray, boundsOnly);
    results.insert(results.end(), nodeResults.begin(), nodeResults.end());

    // Sort and get the closest node
    std::sort(results.begin(), results.end(), [](VROHitTestResult a, VROHitTestResult b) {
        return a.getDistance() < b.getDistance();
    });

    // Return the closest hit element, if any.
    for (int i = 0; i < results.size(); i++) {
        if (!results[i].getNode()->getIgnoreEventHandling()) {
            return results[i];
        }
    }
    
    VROVector3f backgroundPosition = origin + (ray * kSceneBackgroundDistance);
    VROHitTestResult sceneBackgroundHitResult = { sceneRootNode, backgroundPosition,
                                                  kSceneBackgroundDistance, true, camera };
    return sceneBackgroundHitResult;
}

std::shared_ptr<VRONode> VROInputControllerBase::getNodeToHandleEvent(VROEventDelegate::EventAction action,
                                                                      std::shared_ptr<VRONode> node){
    // Base condition, we are asking for the scene's root node's parent, return.
    if (node == nullptr) {
        return nullptr;
    }

    std::shared_ptr<VROEventDelegate> delegate = node->getEventDelegate();
    if (delegate != nullptr && delegate->isEventEnabled(action)){
        return node;
    } else {
        return getNodeToHandleEvent(action, node->getParentNode());
    }
}
