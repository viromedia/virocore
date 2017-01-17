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

void VROInputControllerCardboard::updateScreenTouch(bool onTouchDown){
    if (onTouchDown){
        VROInputControllerBase::onButtonEvent(ViroCardBoard::InputSource::ViewerButton,
                                          VROEventDelegate::ClickState::ClickDown);
    } else {
        VROInputControllerBase::onButtonEvent(ViroCardBoard::InputSource::ViewerButton,
                                          VROEventDelegate::ClickState::ClickUp);
    }
}

void VROInputControllerCardboard::updateOrientation(){
    VROInputControllerBase::onRotate(ViroCardBoard::InputSource::Controller,
                                 _context->getCamera().getRotation());
}

