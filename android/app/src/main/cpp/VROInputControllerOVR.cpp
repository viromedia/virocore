//
//  VROInputControllerOVR.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
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

void VROInputControllerOVR::onProcess(const VROCamera &camera) {
    // Grab controller orientation
    VROQuaternion rotation = camera.getRotation();
    VROVector3f controllerForward = rotation.getMatrix().multiply(kBaseForward);

    // Perform hit test
    VROInputControllerBase::updateHitNode(camera, camera.getPosition(), controllerForward);

    // Process orientation and update delegates
    VROInputControllerBase::onMove(ViroOculus::InputSource::Controller, camera.getPosition(), rotation, controllerForward);
}
