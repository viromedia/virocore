//
//  VROInputControllerCardboard.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include "VROInputControllerCardboard.h"

void VROInputControllerCardboard::onProcess() {
    updateOrientation();
}

void VROInputControllerCardboard::updateScreenTouch(int touchAction){
    VROEventDelegate::ClickState state = touchAction == AMOTION_EVENT_ACTION_DOWN ?
                VROEventDelegate::ClickState::ClickDown : VROEventDelegate::ClickState::ClickUp;
    VROInputControllerBase::onButtonEvent(ViroCardBoard::InputSource::ViewerButton, state);
}

void VROInputControllerCardboard::updateOrientation(){
    // Grab controller orientation
    VROQuaternion rotation = _context->getCamera().getRotation();
    VROVector3f controllerForward = rotation.getMatrix().multiply(kBaseForward);

    // Perform hit test
    VROInputControllerBase::updateHitNode(CONTROLLER_DEFAULT_POSITION, controllerForward);

    // Process orientation and update delegates
    VROInputControllerBase::onMove(ViroCardBoard::InputSource::Controller, CONTROLLER_DEFAULT_POSITION, rotation);
}
