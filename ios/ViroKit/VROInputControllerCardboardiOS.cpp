//
//  VROControllerInputCardboardiOS.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "VROInputControllerCardboardiOS.h"

VROVector3f VROInputControllerCardboardiOS::getDragForwardOffset() {
    // on iOS Cardboard 1) we don't have drag 2) since forward vector of
    // the camera matches the "controller" there's no offset anyways.
    return VROVector3f();
}

void VROInputControllerCardboardiOS::onProcess(const VROCamera &camera) {
    updateOrientation(camera);
    notifyCameraTransform(camera);
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

void VROInputControllerCardboardiOS::updateOrientation(const VROCamera &camera){
    // Grab controller orientation
    VROQuaternion rotation = camera.getRotation();
    VROVector3f controllerForward = rotation.getMatrix().multiply(kBaseForward);
    
    // Perform hit test
    VROInputControllerBase::updateHitNode(camera, camera.getPosition(), controllerForward);
    
    // Process orientation and update delegates
    VROInputControllerBase::onMove(ViroCardBoard::InputSource::Controller, camera.getPosition(), rotation, controllerForward);
    VROInputControllerBase::processGazeEvent(ViroCardBoard::InputSource::Controller);
}

