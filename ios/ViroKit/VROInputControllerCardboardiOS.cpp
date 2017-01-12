//
//  VROControllerInputCardboardiOS.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include "VROInputControllerCardboardiOS.h"

void VROInputControllerCardboardiOS::onProcess() {
    updateOrientation();
}

void VROInputControllerCardboardiOS::onScreenClicked(){
        VROInputControllerBase::onButtonEvent(VROEventDelegate::EventSource::PRIMARY_CLICK,
                                              VROEventDelegate::EventAction::CLICK_UP);
}

void VROInputControllerCardboardiOS::updateOrientation(){
    VROInputControllerBase::onRotate(_context->getCamera().getRotation());
}

