//
//  VROEventDelegate.h
//  ViroRenderer
//
//  Copyright © 2016 Viro Media. All rights reserved.
//

#ifndef VROEventDelegate_h
#define VROEventDelegate_h

#include <stdio.h>
#include <vector>
#include <string>
#include <memory>
#include <set>
#include <map>
#include "VROVector3f.h"

/**
 * Class for both registering for and implementing event delegate callbacks.
 */
class VROEventDelegate {
public:
    /**
     * Enum EventSource types that are supported by this delegate.
     *
     * IMPORTANT: Enum values should match EventSource within EventDelegateJni.java
     * as the standard format to be passed through the JNI layer when enabling /
     * disabling delegate event callbacks. Do Not change the Enum Values!!!
     * Simply add additional event types as need be.
     */
    enum EventSource{
        PRIMARY_CLICK = 1,
        SECONDARY_CLICK = 2,
        VOLUME_UP_CLICK = 3,
        VOLUME_DOWN_CLICK = 4,

        TOUCHPAD_CLICK = 5,
        TOUCHPAD_TOUCH = 6,

        CONTROLLER_GAZE = 7,
        CONTROLLER_ROTATE = 8,
        CONTROLLER_MOVEMENT = 9,
        CONTROLLER_STATUS = 10
    };

    /**
     * Enum EventAction types that are supported by this delegate, used for
     * describing EventSource types. For example, if a click event
     * was CLICK_UP or CLICK_DOWN.
     *
     * IMPORTANT: Enum values should match EventAction within EventDelegateJni.java
     * as the standard format to be passed through the JNI layer.
     * Do Not change the Enum Values!!! Simply add additional event types as need be.
     */
    enum EventAction{
        CLICK_UP = 1,
        CLICK_DOWN = 2,
        GAZE_ON = 3,
        GAZE_OFF = 4,
        TOUCH_ON = 5,
        TOUCH_OFF = 6
    };

    /**
     * Enum ControllerStatus types describing the availability status of the
     * current input controller.
     *
     * IMPORTANT: Enum values should match EventSource within EventDelegateJni.java
     * as the standard format to be passed through the JNI layer.
     * Do Not change the Enum Values!!! Simply add additional event types as need be.
     */
    enum ControllerStatus{
        UNKNOWN = 1,
        CONNECTING = 2,
        CONNECTED = 3,
        DISCONNECTED = 4,
        ERROR = 5
    };

    // Disable all event callbacks by default
    VROEventDelegate(){
        // Controller status
        _enabledEventMap[VROEventDelegate::EventSource::CONTROLLER_STATUS] = false;

        // Orientation events
        _enabledEventMap[VROEventDelegate::EventSource::CONTROLLER_GAZE] = false;
        _enabledEventMap[VROEventDelegate::EventSource::CONTROLLER_ROTATE] = false;
        _enabledEventMap[VROEventDelegate::EventSource::CONTROLLER_MOVEMENT] = false;

        // Button events
        _enabledEventMap[VROEventDelegate::EventSource::PRIMARY_CLICK] = false;
        _enabledEventMap[VROEventDelegate::EventSource::SECONDARY_CLICK] = false;
        _enabledEventMap[VROEventDelegate::EventSource::VOLUME_UP_CLICK] = false;
        _enabledEventMap[VROEventDelegate::EventSource::VOLUME_DOWN_CLICK] = false;

        // Touch pad events
        _enabledEventMap[VROEventDelegate::EventSource::TOUCHPAD_CLICK] = false;
        _enabledEventMap[VROEventDelegate::EventSource::TOUCHPAD_TOUCH] = false;
    }

    /**
     * Informs the renderer to enable / disable the triggering of
     * specific EventSource delegate callbacks.
     */
    void setEnabledEvent(VROEventDelegate::EventSource type, bool enabled){
        _enabledEventMap[type] = enabled;
    }

    bool isEventEnabled(VROEventDelegate::EventSource type){
        return _enabledEventMap[type];
    }

    /*
     * Delegate events triggered by the EventManager.
     */
    virtual void onControllerStatus(ControllerStatus status){
        //No-op
    }

    virtual void onButtonEvent(EventSource type, EventAction event){
        //No-op
    }

    virtual void onTouchPadEvent(EventSource type, EventAction event, float x, float y){
        //No-op
    }

    virtual void onRotate(VROVector3f rotation) {
        //No-op
    }

    virtual void onPosition(VROVector3f location) {
        //No-op
    }

    virtual void onGaze(bool isGazing){
        //No-op
    }

    virtual void onGazeHit(float distance, VROVector3f hitLocation){
        //No-op
    }
private:
    std::map<VROEventDelegate::EventSource, bool> _enabledEventMap;
};
#endif