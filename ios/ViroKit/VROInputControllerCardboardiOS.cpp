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
    // As ios doesn't have separate down up events, we simulate a button click
    // by triggering both click down / click up.
    VROInputControllerBase::onButtonEvent(ViroCardBoard::ViewerButton, VROEventDelegate::ClickState::ClickDown);
    VROInputControllerBase::onButtonEvent(ViroCardBoard::ViewerButton, VROEventDelegate::ClickState::ClickUp);
}

void VROInputControllerCardboardiOS::updateOrientation(){
    VROInputControllerBase::onRotate(ViroCardBoard::Controller,
                                     _context->getCamera().getRotation());
}

