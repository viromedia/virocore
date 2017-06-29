//
//  VROInputControllerCardboard.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include "VROInputControllerCardboard.h"

VROVector3f VROInputControllerCardboard::getDragForwardOffset() {
    // since the controller is the same as the camera, there's no offset.
    return VROVector3f();
}

void VROInputControllerCardboard::onProcess(const VROCamera &camera) {
    updateOrientation(camera);
}

void VROInputControllerCardboard::updateScreenTouch(int touchAction){
    VROEventDelegate::ClickState state = touchAction == AMOTION_EVENT_ACTION_DOWN ?
                VROEventDelegate::ClickState::ClickDown : VROEventDelegate::ClickState::ClickUp;
    VROInputControllerBase::onButtonEvent(ViroCardBoard::InputSource::ViewerButton, state);
}

void VROInputControllerCardboard::updateOrientation(const VROCamera &camera){
    // Grab controller orientation
    VROQuaternion rotation = camera.getRotation();
    VROVector3f controllerForward = rotation.getMatrix().multiply(kBaseForward);

    // Perform hit test
    VROInputControllerBase::updateHitNode(camera, camera.getPosition(), controllerForward);

    // Process orientation and update delegates
    VROInputControllerBase::onMove(ViroCardBoard::InputSource::Controller, camera.getPosition(), rotation, controllerForward);
}
