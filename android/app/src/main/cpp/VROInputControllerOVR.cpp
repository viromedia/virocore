//
//  VROInputControllerOVR.cpp
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

#include "VROInputControllerOVR.h"

void VROInputControllerOVR::handleOVRTouchEvent(int touchAction, float posX, float posY){
    VROEventDelegate::TouchState action;
    if ( touchAction == AMOTION_EVENT_ACTION_UP){
        action = VROEventDelegate::TouchState::TouchUp;
        VROInputControllerBase::onButtonEvent(ViroOculus::TouchPad, VROEventDelegate::ClickState::ClickUp);
        updateSwipeGesture(_touchDownLocationStart, VROVector3f(posX, posY, 0));
    } else if ( touchAction == AMOTION_EVENT_ACTION_DOWN){
        action = VROEventDelegate::TouchState::TouchDown;
        _touchDownLocationStart = VROVector3f(posX, posY, 0);
        VROInputControllerBase::onButtonEvent(ViroOculus::TouchPad, VROEventDelegate::ClickState::ClickDown);
    }  else if ( touchAction == AMOTION_EVENT_ACTION_MOVE){
        action = VROEventDelegate::TouchState::TouchDownMove;
    } else {
        return;
    }

    VROInputControllerBase::onTouchpadEvent(ViroOculus::InputSource::TouchPad, action, posX, posY);
}

void VROInputControllerOVR::updateSwipeGesture(VROVector3f start, VROVector3f end){
    VROVector3f diff = end - start;
    float xDist = fabs(diff.x);
    float yDist = fabs(diff.y);
    VROEventDelegate::SwipeState swipeState;
    if (xDist > yDist){
        if (diff.x > 0){
            swipeState = VROEventDelegate::SwipeState::SwipeLeft;
        } else {
            swipeState = VROEventDelegate::SwipeState::SwipeRight;
        }
    } else {
        if (diff.y > 0){
            swipeState = VROEventDelegate::SwipeState::SwipeDown;
        } else {
            swipeState = VROEventDelegate::SwipeState::SwipeUp;
        }
    }
    VROInputControllerBase::onSwipe(ViroOculus::InputSource::TouchPad, swipeState);
}

void VROInputControllerOVR::handleOVRKeyEvent(int keyCode, int action){
    VROEventDelegate::ClickState state = action == AKEY_EVENT_ACTION_DOWN ?
                                         VROEventDelegate::ClickState::ClickDown :  VROEventDelegate::ClickState::ClickUp;
    if (keyCode == AKEYCODE_BACK ) {
        VROInputControllerBase::onButtonEvent(ViroOculus::BackButton, state);
    }
}

VROVector3f VROInputControllerOVR::getDragForwardOffset() {
    return (_hitResult->getLocation() - _lastKnownPosition).normalize() - _lastKnownForward;
}

void VROInputControllerOVR::onProcess(const VROCamera &camera) {
    // Grab controller orientation
    VROQuaternion rotation = camera.getRotation();
    VROVector3f controllerForward = rotation.getMatrix().multiply(kBaseForward);

    // Perform hit test
    VROInputControllerBase::updateHitNode(camera, camera.getPosition(), controllerForward);

    // Process orientation and update delegates
    VROInputControllerBase::onMove(ViroOculus::InputSource::Controller, camera.getPosition(), rotation, controllerForward);
    VROInputControllerBase::processGazeEvent(ViroOculus::InputSource::Controller);

    notifyCameraTransform(camera);
}
