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
    } else if ( touchAction == AMOTION_EVENT_ACTION_DOWN){
        action = VROEventDelegate::TouchState::TouchDown;
        VROInputControllerBase::onButtonEvent(ViroOculus::TouchPad, VROEventDelegate::ClickState::ClickDown);
    }  else if ( touchAction == AMOTION_EVENT_ACTION_MOVE){
        action = VROEventDelegate::TouchState::TouchDownMove;
    } else {
        return;
    }

    VROInputControllerBase::onTouchpadEvent(ViroDayDream::InputSource::TouchPad, action, posX, posY);
}

void VROInputControllerOVR::handleOVRKeyEvent(int keyCode, int action){
    VROEventDelegate::ClickState state = action == AKEY_EVENT_ACTION_DOWN ?
                                         VROEventDelegate::ClickState::ClickDown :  VROEventDelegate::ClickState::ClickUp;
    if (keyCode == AKEYCODE_BACK ) {
        VROInputControllerBase::onButtonEvent(ViroOculus::BackButton, state);
    }
}

void VROInputControllerOVR::onProcess() {
    VROInputControllerBase::onRotate(ViroOculus::Controller,
                                     _context->getCamera().getRotation());
}
