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
    // Grab controller orientation
    VROQuaternion rotation = _context->getCamera().getRotation();
    VROVector3f controllerForward = rotation.getMatrix().multiply(kBaseForward);
    
    // Perform hit test
    VROInputControllerBase::updateHitNode(CONTROLLER_DEFAULT_POSITION, controllerForward);
    
    // Process orientation and update delegates
    VROInputControllerBase::onMove(ViroCardBoard::InputSource::Controller, CONTROLLER_DEFAULT_POSITION, rotation);



}

