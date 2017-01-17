//
//  VROInputControllerDaydream.cpp
//  ViroRenderer
//
//  Copyright © 2017 Viro Media. All rights reserved.
//
#include "VROInputControllerDaydream.h"

VROInputControllerDaydream::VROInputControllerDaydream(gvr_context *gvr_context) :
    _gvr_controller(new gvr::ControllerApi) {
    // Initialize default options for the controller API.
    int32_t options = gvr::ControllerApi::DefaultOptions();
    _hasInitalized = _gvr_controller->Init(options, gvr_context);
    if (!_hasInitalized){
        perror("Error: Failure to initialize DayDream Controller!");
    }
}

VROInputControllerDaydream::~VROInputControllerDaydream() {}

void VROInputControllerDaydream::onProcess() {
    /**
     * Do not proceed in case of failure (calling other controller_api methods
     * without a successful Init will crash with an assert failure.
     */
    if (!_hasInitalized){
      return;
    }
    _controller_state.Update(*_gvr_controller);

    if (!isControllerReady()){
        return;
    }

    // Update all the controller input states
    updateOrientation();
    updateButtons();
    updateTouchPad();
}

void VROInputControllerDaydream::updateButtons() {
    notifyButtonEventForType(GVR_CONTROLLER_BUTTON_CLICK, ViroDayDream::InputSource::TouchPad);
    notifyButtonEventForType(GVR_CONTROLLER_BUTTON_APP, ViroDayDream::InputSource::AppButton);
    notifyButtonEventForType(GVR_CONTROLLER_BUTTON_VOLUME_UP, ViroDayDream::InputSource::VolUpButton);
    notifyButtonEventForType(GVR_CONTROLLER_BUTTON_VOLUME_DOWN, ViroDayDream::InputSource::VolDownButton);
}

void VROInputControllerDaydream::notifyButtonEventForType(gvr::ControllerButton button, ViroDayDream::InputSource source) {
    if (_controller_state.GetButtonDown(button)){
        VROInputControllerBase::onButtonEvent(source, VROEventDelegate::ClickState::ClickDown);
    } else if (_controller_state.GetButtonUp(button)){
        VROInputControllerBase::onButtonEvent(source, VROEventDelegate::ClickState::ClickUp);
    }
}

void VROInputControllerDaydream::updateTouchPad() {
    float posX = _controller_state.GetTouchPos().x;
    float posY =  _controller_state.GetTouchPos().y;

    VROEventDelegate::TouchState action;
    if (_controller_state.GetTouchUp()){
        action = VROEventDelegate::TouchState::TouchUp;
    } else if (_controller_state.GetTouchDown()){
        action = VROEventDelegate::TouchState::TouchDown;
    }  else if (_controller_state.IsTouching()){
        action = VROEventDelegate::TouchState::TouchDownMove;
    } else {
        return;
    }

    VROInputControllerBase::onTouchpadEvent(ViroDayDream::InputSource::TouchPad, action, posX, posY);
}

void VROInputControllerDaydream::updateOrientation(){
    gvr_quatf rotation = _controller_state.GetOrientation();
    VROInputControllerBase::onRotate(ViroDayDream::InputSource::Controller,
                                     VROQuaternion(rotation.qx, rotation.qy, rotation.qz, rotation.qw));
}

bool VROInputControllerDaydream::isControllerReady(){
    // Check Controller API status
    gvr::ControllerApiStatus apiStatus = _controller_state.GetApiStatus();
    switch (apiStatus) {
        case GVR_CONTROLLER_API_OK:
            break;
        default:
            VROInputControllerBase::onControllerStatus(ViroDayDream::InputSource::Controller,
                                                       VROEventDelegate::ControllerStatus::Error);
            return false;
    }

    // Check Controller Connectivity status
    gvr::ControllerConnectionState connState = _controller_state.GetConnectionState();
    switch (connState) {
        case GVR_CONTROLLER_CONNECTED:
            VROInputControllerBase::onControllerStatus(ViroDayDream::InputSource::Controller,
                                                       VROEventDelegate::ControllerStatus::Connected);
            return true;
        case GVR_CONTROLLER_SCANNING:
        case GVR_CONTROLLER_CONNECTING:
            VROInputControllerBase::onControllerStatus(ViroDayDream::InputSource::Controller,
                                                       VROEventDelegate::ControllerStatus::Connecting);
            return false;
        default:
            VROInputControllerBase::onControllerStatus(ViroDayDream::InputSource::Controller,
                                                       VROEventDelegate::ControllerStatus::Disconnected);
            return false;
    }
}

void VROInputControllerDaydream::onPause() {
    _gvr_controller->Pause();
}

void VROInputControllerDaydream::onResume() {
    _gvr_controller->Resume();
}

