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

std::string VROInputControllerCardboardiOS::getHeadset() {
  return std::string("cardboard");
}

std::string VROInputControllerCardboardiOS::getController() {
  return std::string("cardboard");
}

void VROInputControllerCardboardiOS::updateOrientation(){
    VROInputControllerBase::onRotate(ViroCardBoard::Controller,
                                     _context->getCamera().getRotation());
    VROInputControllerBase::updateHitNode(_lastKnownPosition, _lastKnownForward);
    VROInputControllerBase::notifyOrientationDelegates(ViroCardBoard::Controller);
}

