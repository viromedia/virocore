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
        VROInputControllerBase::onButtonEvent(VROEventDelegate::EventSource::PRIMARY_CLICK,
                                              VROEventDelegate::EventAction::CLICK_DOWN);
    } else {
        VROInputControllerBase::onButtonEvent(VROEventDelegate::EventSource::PRIMARY_CLICK,
                                              VROEventDelegate::EventAction::CLICK_UP);
    }
}

void VROInputControllerCardboard::updateOrientation(){
    VROInputControllerBase::onRotate(_context->getCamera().getRotation());
}

