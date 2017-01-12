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
        perror("ERROR: Failure to initialize DayDream Controller!");
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
    notifyButtonEventForType(GVR_CONTROLLER_BUTTON_CLICK, VROEventDelegate::EventSource::PRIMARY_CLICK);
    notifyButtonEventForType(GVR_CONTROLLER_BUTTON_APP, VROEventDelegate::EventSource::SECONDARY_CLICK);
    notifyButtonEventForType(GVR_CONTROLLER_BUTTON_VOLUME_UP, VROEventDelegate::EventSource::VOLUME_UP_CLICK);
    notifyButtonEventForType(GVR_CONTROLLER_BUTTON_VOLUME_DOWN, VROEventDelegate::EventSource::VOLUME_DOWN_CLICK);
}

void VROInputControllerDaydream::notifyButtonEventForType(gvr::ControllerButton button, VROEventDelegate::EventSource type) {
    if (_controller_state.GetButtonDown(button)){
        VROInputControllerBase::onButtonEvent(type, VROEventDelegate::EventAction::CLICK_DOWN);
    } else if (_controller_state.GetButtonUp(button)){
        VROInputControllerBase::onButtonEvent(type, VROEventDelegate::EventAction::CLICK_UP);
    }
}

void VROInputControllerDaydream::updateTouchPad() {
    float lastTouchedPos_x = _controller_state.GetTouchPos().x;
    float lastTouchPos_y =  _controller_state.GetTouchPos().y;

    VROEventDelegate::EventAction action;
    VROEventDelegate::EventSource source;
    if (_controller_state.GetButtonUp(GVR_CONTROLLER_BUTTON_CLICK)){
        action = VROEventDelegate::EventAction::CLICK_UP;
        source = VROEventDelegate::EventSource::TOUCHPAD_CLICK;
    } else if (_controller_state.GetButtonDown(GVR_CONTROLLER_BUTTON_CLICK)){
        action = VROEventDelegate::EventAction::CLICK_DOWN;
        source = VROEventDelegate::EventSource::TOUCHPAD_CLICK;
    } else if (_controller_state.GetTouchUp()){
        action = VROEventDelegate::EventAction::TOUCH_OFF;
        source = VROEventDelegate::EventSource::TOUCHPAD_TOUCH;
    } else if (_controller_state.IsTouching()){
        action = VROEventDelegate::EventAction::TOUCH_ON;
        source = VROEventDelegate::EventSource::TOUCHPAD_TOUCH;
    } else {
        return;
    }

    VROInputControllerBase::onTouchpadEvent(source, action, lastTouchedPos_x, lastTouchPos_y);
}

void VROInputControllerDaydream::updateOrientation(){
    gvr_quatf rotation = _controller_state.GetOrientation();
    VROInputControllerBase::onRotate(VROQuaternion(rotation.qx, rotation.qy, rotation.qz, rotation.qw));
}

bool VROInputControllerDaydream::isControllerReady(){
    // Check Controller API status
    gvr::ControllerApiStatus apiStatus = _controller_state.GetApiStatus();
    switch (apiStatus) {
        case GVR_CONTROLLER_API_OK:
            break;
        default:
            VROInputControllerBase::onControllerStatus(VROEventDelegate::ControllerStatus::ERROR);
            return false;
    }

    // Check Controller Connectivity status
    gvr::ControllerConnectionState connState = _controller_state.GetConnectionState();
    switch (connState) {
        case GVR_CONTROLLER_CONNECTED:
            VROInputControllerBase::onControllerStatus(VROEventDelegate::ControllerStatus::CONNECTED);
            return true;
        case GVR_CONTROLLER_SCANNING:
        case GVR_CONTROLLER_CONNECTING:
            VROInputControllerBase::onControllerStatus(VROEventDelegate::ControllerStatus::CONNECTING);
            return false;
        default:
            VROInputControllerBase::onControllerStatus(VROEventDelegate::ControllerStatus::DISCONNECTED);
            return false;
    }
}

void VROInputControllerDaydream::onPause() {
    _gvr_controller->Pause();
}

void VROInputControllerDaydream::onResume() {
    _gvr_controller->Resume();
}

