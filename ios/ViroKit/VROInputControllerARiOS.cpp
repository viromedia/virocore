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
    _isPinchOngoing(false) {
}

VROVector3f VROInputControllerARiOS::getDragForwardOffset() {
    return VROVector3f();
}

void VROInputControllerARiOS::onProcess(const VROCamera &camera) {
    _latestCamera = camera;
    processTouchMovement();
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
            VROARHitTestResult result = findBestHitTestResult(results);
            VROVector3f position = result.getWorldTransform().extractTranslation();
            
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
            VROTransaction::setAnimationDuration(.075);
            arDraggableNode->setPosition(position);
            std::weak_ptr<VROARDraggableNode> weakNode = arDraggableNode;
            VROTransaction::setFinishCallback([weakNode]() {
                std::shared_ptr<VROARDraggableNode> strongNode = weakNode.lock();
                if (strongNode) {
                    strongNode->setAnimating(false);
                }
            });
            
            // Update last known dragged position and notify delegates
            _lastDraggedNodePosition = position;
            
            arDraggableNode->getEventDelegate()->onDrag(source, position);
            for (std::shared_ptr<VROEventDelegate> delegate : _delegates) {
                delegate->onDrag(source, position);
            }
        }
    }
}

VROARHitTestResult VROInputControllerARiOS::findBestHitTestResult(std::vector<VROARHitTestResult> results) {
    VROARHitTestResult toReturn = results[0];
    for (int i = 1; i < results.size(); i++) {
        VROARHitTestResult candidate = results[i];
        if (valueForHitTestResultType(candidate.getType()) > valueForHitTestResultType(toReturn.getType())) {
            toReturn = candidate;
        }
    }
    return toReturn;
}

/*
 This function takes a VROARHitTestResultType and assigns an int value to them where
 larger int values are "preferred" over smaller ones.
 */
int VROInputControllerARiOS::valueForHitTestResultType(VROARHitTestResultType type) {
    switch(type) {
        case VROARHitTestResultType::ExistingPlaneUsingExtent:
            return 3;
        case VROARHitTestResultType::ExistingPlane:
            return 1;
        case VROARHitTestResultType::EstimatedHorizontalPlane:
            return 0;
        case VROARHitTestResultType::FeaturePoint:
            return 2;
    }
}

std::string VROInputControllerARiOS::getHeadset() {
    return std::string("Mobile");
}

std::string VROInputControllerARiOS::getController() {
    return std::string("Screen");
}

void VROInputControllerARiOS::processTouchMovement() {
  if(_isPinchOngoing) {
      VROInputControllerBase::onPinch(ViroCardBoard::InputSource::Controller, _latestScale, VROEventDelegate::PinchState::PinchMove);
  }else if (_isTouchOngoing) {
      VROVector3f rayFromCamera = calculateCameraRay(_latestTouchPos);
      VROInputControllerBase::updateHitNode(_latestCamera, _latestCamera.getPosition(), rayFromCamera);
      VROInputControllerBase::onMove(ViroCardBoard::InputSource::Controller, _latestCamera.getPosition(), _latestCamera.getRotation(), rayFromCamera);
    }
}

/* TODO: VIRO-1465 fix this function to work properly.
 
 This is slightly unconventional because what I'm doing is "unprojecting" the
 touch position back into camera space. Then the ray out is simply that unprojected
 point minus the camera position (which is 0,0,0 in camera space). Then I just take
 that ray and "rotate" it by the camera's rotation back into world coordinates with
 the assumed origin at the camera's position. This does give the same value as if I
 calculate far - near and use that, but the values are off (more obviously wrong?) on
 tablets, so I suspect an issue with a matrix or something.
 
VROVector3f VROInputControllerARiOS::calculateCameraRay(VROVector3f touchPos) {
    std::shared_ptr<VRORenderer> renderer = _weakRenderer.lock();
    if (!renderer) {
        return VROVector3f();
    }
    
    int viewportArr[4] = {0, 0, (int) _viewportWidth, (int) _viewportHeight};
    
    // calculate the mvp matrix
    VROMatrix4f projectionMat = renderer->getRenderContext()->getProjectionMatrix();
    VROMatrix4f viewMat = renderer->getRenderContext()->getViewMatrix();
    VROMatrix4f modelMat = _latestCamera.getRotation().getMatrix();
    modelMat.translate(_latestCamera.getPosition());
    
    VROMatrix4f mvp = modelMat.multiply(viewMat).multiply(projectionMat);
    
    // unproject the touchPos vector (w/ z = 0) from viewport coords to camera coords
    VROVector3f resultNear;
    
    VROProjector::unproject(VROVector3f(touchPos.x, touchPos.y), mvp.getArray(), viewportArr, &resultNear);
    
    // since we want the ray "from" the camera position in the world, rotate it back to world coordinates (but don't translate).
    return _latestCamera.getRotation().getMatrix().multiply(resultNear).normalize();
}
 */

/*
 This is a stop-gap measure while we fix the above function which leverages ARKit's hitTest to
 give us some point based on tap location from which we simply normalize and present that as the
 forward camera ray.
 */
VROVector3f VROInputControllerARiOS::calculateCameraRay(VROVector3f touchPos) {
    std::shared_ptr<VROARSession> session = _weakSession.lock();
    if (session) {
        std::unique_ptr<VROARFrame> &frame = session->getLastFrame();
        std::vector<VROARHitTestResult> results = frame->hitTest(_latestTouchPos.x,
                                                                 _latestTouchPos.y,
                                                                 { VROARHitTestResultType::ExistingPlaneUsingExtent,
                                                                     VROARHitTestResultType::ExistingPlane,
                                                                     VROARHitTestResultType::EstimatedHorizontalPlane,
                                                                     VROARHitTestResultType::FeaturePoint });
        
        if (results.size() > 0) {
            // just grab the first result, we don't care about accuracy because all points returned are
            // along the same line.
            VROARHitTestResult result = results[0];
            VROVector3f position = result.getWorldTransform().extractTranslation();
            VROVector3f ray = (position - _latestCamera.getPosition()).normalize();
            
            return ray;
        }
    }
    return VROVector3f();
}
