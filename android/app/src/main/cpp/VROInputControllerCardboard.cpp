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
    VROInputControllerBase::onRotate(ViroCardBoard::InputSource::Controller,
                                 _context->getCamera().getRotation());
}

