//
//  VROInputControllerARiOS.cpp
//  ViroKit
//
//  Created by Andy Chu on 6/21/17.
//  Copyright © 2017 Viro Media. All rights reserved.
//

#include "VROInputControllerARiOS.h"
#include "VRORenderer.h"
#include "VROProjector.h"
#include "VROARFrame.h"
#include "VROARDraggableNode.h"

VROInputControllerARiOS::VROInputControllerARiOS(float viewportWidth, float viewportHeight) :
    _viewportWidth(viewportWidth),
    _viewportHeight(viewportHeight),
    _isTouchOngoing(false),
    _isPinchOngoing(false),
    _isRotateOngoing(false) {
}

VROVector3f VROInputControllerARiOS::getDragForwardOffset() {
    return VROVector3f();
}

void VROInputControllerARiOS::onProcess(const VROCamera &camera) {
    _latestCamera = camera;
    processTouchMovement();
}

void VROInputControllerARiOS::onRotateStart(VROVector3f touchPos) {
    _isRotateOngoing = true;
    VROVector3f rayFromCamera = calculateCameraRay(touchPos);
    VROInputControllerBase::updateHitNode(_latestCamera, _latestCamera.getPosition(), rayFromCamera);
    VROInputControllerBase::onRotate(ViroCardBoard::InputSource::Controller, 1.0, VROEventDelegate::RotateState::RotateStart);
}


void VROInputControllerARiOS::onRotate(float rotation) {
    _latestRotation = rotation;
}

void VROInputControllerARiOS::onRotateEnd() {
    _isRotateOngoing = false;
    VROInputControllerBase::onRotate(ViroCardBoard::InputSource::Controller, _latestRotation, VROEventDelegate::RotateState::RotateEnd);
}

void VROInputControllerARiOS::onPinchStart(VROVector3f touchPos) {
    _isPinchOngoing = true;
    VROVector3f rayFromCamera = calculateCameraRay(touchPos);
    VROInputControllerBase::updateHitNode(_latestCamera, _latestCamera.getPosition(), rayFromCamera);
    VROInputControllerBase::onPinch(ViroCardBoard::InputSource::Controller, 1.0, VROEventDelegate::PinchState::PinchStart);
}


void VROInputControllerARiOS::onPinchScale(float scale) {
    _latestScale = scale;
}

void VROInputControllerARiOS::onPinchEnd() {
    _isPinchOngoing = false;
    VROInputControllerBase::onPinch(ViroCardBoard::InputSource::Controller, _latestScale, VROEventDelegate::PinchState::PinchEnd);
}

void VROInputControllerARiOS::onScreenTouchDown(VROVector3f touchPos) {
    _latestTouchPos = touchPos;
    _isTouchOngoing = true;
    VROVector3f rayFromCamera = calculateCameraRay(_latestTouchPos);
    VROInputControllerBase::updateHitNode(_latestCamera, _latestCamera.getPosition(), rayFromCamera);
    VROInputControllerBase::onButtonEvent(ViroCardBoard::ViewerButton, VROEventDelegate::ClickState::ClickDown);
}

void VROInputControllerARiOS::onScreenTouchMove(VROVector3f touchPos) {
    _latestTouchPos = touchPos;
}

void VROInputControllerARiOS::onScreenTouchUp(VROVector3f touchPos) {
    _latestTouchPos = touchPos;
    _isTouchOngoing = false;
    VROVector3f rayFromCamera = calculateCameraRay(_latestTouchPos);
    VROInputControllerBase::updateHitNode(_latestCamera, _latestCamera.getPosition(), rayFromCamera);
    VROInputControllerBase::onButtonEvent(ViroCardBoard::ViewerButton, VROEventDelegate::ClickState::ClickUp);
    
    // on touch up, we should invoke processDragging once more in case
    if (_lastDraggedNode) {
        // in AR, source is always the controller.
        processDragging(ViroCardBoard::InputSource::Controller, true);
    }
}

void VROInputControllerARiOS::processDragging(int source) {
    processDragging(source, false);
}

void VROInputControllerARiOS::processDragging(int source, bool alwaysRun) {
    std::shared_ptr<VROARDraggableNode> arDraggableNode = std::dynamic_pointer_cast<VROARDraggableNode>(_lastDraggedNode->_draggedNode);
    if (!arDraggableNode) {
        VROInputControllerBase::processDragging(source);
        return;
    }
    
    std::shared_ptr<VROARSession> session = _weakSession.lock();
    // only process AR drag if we have a session and we've waited long enough since the last time we processed drag OR
    // if alwaysRun is true.
    if ((session && (VROTimeCurrentMillis() - _lastProcessDragTimeMillis > kARProcessDragInterval)) || alwaysRun) {
        std::unique_ptr<VROARFrame> &frame = session->getLastFrame();
        std::vector<VROARHitTestResult> results = frame->hitTest(_latestTouchPos.x,
                                                                 _latestTouchPos.y,
                                                                 { VROARHitTestResultType::ExistingPlaneUsingExtent,
                                                                     VROARHitTestResultType::ExistingPlane,
                                                                     VROARHitTestResultType::EstimatedHorizontalPlane,
                                                                     VROARHitTestResultType::FeaturePoint });
        
        if (results.size() > 0) {
            VROVector3f position = getNextDragPosition(results);
            
            // TODO: since we're animating position... the position passed back below won't necessarily
            // reflect its real position.
            /*
             * To avoid spamming the JNI / JS bridge, throttle the notification
             * of onDrag delegates to a certain degree of accuracy.
             */
            float distance = position.distance(_lastDraggedNodePosition);
            if (distance < ON_DRAG_DISTANCE_THRESHOLD) {
                return;
            }
            
            // if we're already animating, then "cancel" it at the current position vs terminating
            // which causes the object to "jump" to the end.
            if (arDraggableNode->isAnimating() && arDraggableNode->getAnimationTransaction()) {
                VROTransaction::cancel(arDraggableNode->getAnimationTransaction());
                arDraggableNode->setAnimating(false);
            }
            
            // create new transaction to the new location
            VROTransaction::begin();
            VROTransaction::setAnimationDuration(.1);
            arDraggableNode->setPosition(position);
            std::weak_ptr<VROARDraggableNode> weakNode = arDraggableNode;
            VROTransaction::setFinishCallback([weakNode]() {
                std::shared_ptr<VROARDraggableNode> strongNode = weakNode.lock();
                if (strongNode) {
                    strongNode->setAnimating(false);
                }
            });
            
            // Update last known dragged position, distance to controller and notify delegates
            _lastDraggedNodePosition = position;
            _lastDraggedNode->_draggedDistanceFromController = position.distanceAccurate(_latestCamera.getPosition());
            
            arDraggableNode->getEventDelegate()->onDrag(source, position);
            for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
                delegate->onDrag(source, position);
            }
        }
    }
}

/*
 Things we're doing to select the next drag position:
 - picking plane with extent if possible
 - looking at all feature points
     - picking the feature point that changed the least from the last position
     - ensuring that minDist < feature point < maxDist
     - ensuring that feature point is in same general direction as forward (in front of the camera)
 - picking plane without extent if no feature points
     - ensuring that plane without extent also is between minDist and maxDist
 - finally, if all else fails, we should always get at least something back from ARKit
     - grab that hit result, normalize it and multiply it by the last distance we used

 Things we aren't doing but could:
 - ignoring ExistingPlaneUsingExtent because it causes us to miss smaller objects (think
   trashcans/chairs) when a plane extends under the objects.
 - Smoothing the position - this might not be ideal in certain cases where we're certain of
   the position because if you drag an object from the table to the floor, there's no intermediary position.
 - Parse the point cloud ourselves to more directly influence the estimates.
 - "Shotgun" the area with hit tests to find the best position.
 */
VROVector3f VROInputControllerARiOS::getNextDragPosition(std::vector<VROARHitTestResult> results) {
    VROVector3f cameraPos = _latestCamera.getPosition();
    
    // first, filter the points, if we find an ExistingPlaneUsingExtent, then return that (highest confidence)
    std::vector<VROARHitTestResult> featurePoints;
    std::shared_ptr<VROARHitTestResult> existingPlaneWithoutExtent = nullptr;
    for (VROARHitTestResult result : results) {
        switch (result.getType()) {
            case VROARHitTestResultType::ExistingPlaneUsingExtent:
                if (isDistanceWithinBounds(cameraPos, result.getWorldTransform().extractTranslation())) {
                    return result.getWorldTransform().extractTranslation();
                }
                break;
            case VROARHitTestResultType::ExistingPlane:
                existingPlaneWithoutExtent = std::make_shared<VROARHitTestResult>(result);
                break;
            case VROARHitTestResultType::FeaturePoint:
                featurePoints.push_back(result);
                break;
            }
    }
    
    // Take the given feature points and find the one that changed the least from the last point.
    if (featurePoints.size() > 0) {
        std::sort(featurePoints.begin(), featurePoints.end(), [this](VROARHitTestResult a, VROARHitTestResult b) {
            VROVector3f posA = a.getWorldTransform().extractTranslation();
            VROVector3f posB = b.getWorldTransform().extractTranslation();
            float distALast = posA.distance(this->_lastDraggedNodePosition);
            float distBLast = posB.distance(this->_lastDraggedNodePosition);
            return distALast < distBLast;
        });

        for (VROARHitTestResult featurePoint : featurePoints) {
            VROVector3f featurePointPos = featurePoint.getWorldTransform().extractTranslation();
            VROVector3f ray = featurePointPos - _latestCamera.getPosition();
            if (isDistanceWithinBounds(cameraPos, featurePointPos) && _latestCamera.getForward().dot(ray) > 0) {
                return featurePointPos;
            }
        }
    }

    // if we don't trust/have any feature points, then use the existingPlaneWithoutExtent.
    if (existingPlaneWithoutExtent) {
        VROVector3f planePos = existingPlaneWithoutExtent->getWorldTransform().extractTranslation();
        if (isDistanceWithinBounds(cameraPos, planePos)) {
            return planePos;
        }
    }

    float distance = _lastDraggedNode->_draggedDistanceFromController;
    distance = fmin(distance, kARMaxDragDistance);
    distance = fmax(distance, kARMinDragDistance);
    VROVector3f touchForward = (results[0].getWorldTransform().extractTranslation() - cameraPos).normalize();
    
    // sometimes the touch ray is calculated "behind" the camera forward, so just flip it.
    float projection = _latestCamera.getForward().dot(touchForward);
    if (projection < 0) {
        touchForward = touchForward * -1;
    }
    
    return cameraPos + (touchForward * distance);
}

bool VROInputControllerARiOS::isDistanceWithinBounds(VROVector3f point1, VROVector3f point2) {
    float distance = point1.distance(point2);
    return distance > kARMinDragDistance && distance < kARMaxDragDistance;
}

std::string VROInputControllerARiOS::getHeadset() {
    return std::string("Mobile");
}

std::string VROInputControllerARiOS::getController() {
    return std::string("Screen");
}

void VROInputControllerARiOS::processTouchMovement() {
    if (_isPinchOngoing || _isRotateOngoing) {
        if(_isPinchOngoing) {
            VROInputControllerBase::onPinch(ViroCardBoard::InputSource::Controller, _latestScale, VROEventDelegate::PinchState::PinchMove);
        } else {
             VROInputControllerBase::onRotate(ViroCardBoard::InputSource::Controller, _latestRotation, VROEventDelegate::RotateState::RotateMove);
        }
    } else if (_isTouchOngoing) {
        VROVector3f rayFromCamera = calculateCameraRay(_latestTouchPos);
        VROInputControllerBase::updateHitNode(_latestCamera, _latestCamera.getPosition(), rayFromCamera);
        VROInputControllerBase::onMove(ViroCardBoard::InputSource::Controller, _latestCamera.getPosition(), _latestCamera.getRotation(), rayFromCamera);
    }
}
 
VROVector3f VROInputControllerARiOS::calculateCameraRay(VROVector3f touchPos) {
    std::shared_ptr<VRORenderer> renderer = _weakRenderer.lock();
    if (!renderer) {
        return VROVector3f();
    }
    
    int viewportArr[4] = {0, 0, (int) _viewportWidth, (int) _viewportHeight};
    
    // calculate the mvp matrix
    VROMatrix4f projectionMat = renderer->getRenderContext()->getProjectionMatrix();
    VROMatrix4f viewMat = renderer->getRenderContext()->getViewMatrix();
    
    VROMatrix4f mvp = projectionMat.multiply(viewMat);
    
    // unproject the touchPos vector at z = 0 and z = 1
    VROVector3f resultNear;
    VROVector3f resultFar;
    
    VROProjector::unproject(VROVector3f(touchPos.x, touchPos.y, 0), mvp.getArray(), viewportArr, &resultNear);
    VROProjector::unproject(VROVector3f(touchPos.x, touchPos.y, 1), mvp.getArray(), viewportArr, &resultFar);
    
    // minus resultNear from resultFar to get the vector.
    return (resultFar - resultNear).normalize();
}

