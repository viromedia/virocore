/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import java.util.HashMap;
import java.util.Map;

/**
 * Java JNI wrapper responsible for registering and implementing event
 * delegate callbacks across the JNI bridge. It ultimately links the
 * following classes below.
 *
 * Java JNI Wrapper     : com.viro.renderer.EventDelegateJni.java
 * Cpp JNI wrapper      : EventDelegate_JNI.cpp
 * Cpp Object           : VROEventDelegate.cpp
 */
public class EventDelegateJni {
    final protected long mNativeRef;
    private EventDelegateCallback mDelegate = null;

    public EventDelegateJni() {
        mNativeRef = nativeCreateDelegate();
    }

    public void destroy() {
        nativeDestroyDelegate(mNativeRef);
    }

    /**
     * Enables or disables the given event type. If enabled, the callback corresponding to
     * the even type is triggered, else, disabled otherwise. Note: All EventTypes are
     * disabled by default.
     */
    public void setEventEnabled(EventSource type, boolean enabled) {
        nativeEnableEvent(mNativeRef, type.mTypeId, enabled);
    }

    public void setEventDelegateCallback(EventDelegateCallback delegate) {
        mDelegate = delegate;
    }

    /**
     * Native Functions called into JNI
     */
    private native long nativeCreateDelegate();
    private native void nativeDestroyDelegate(long mNativeNodeRef);
    private native void nativeEnableEvent(long mNativeNodeRef, int eventType, boolean enabled);

    /**
     * EventSource types corresponding to VROEventDelegate.h, used for enabling or
     * disabling delegate event callbacks through the JNI layer.
     *
     * EventTypes used in Java are mapped to the function callbacks within
     * EventDelegateCallbacks.
     *
     * IMPORTANT: Do Not change the Enum Values!!! Simply add additional
     * event types as need be. This should always map directly to
     * VROEventDelegate.h
     */
    public enum EventSource {
        // Button events
        PRIMARY_CLICK(1),
        SECONDARY_CLICK(2),
        VOLUME_UP_CLICK(3),
        VOLUME_DOWN_CLICK(4),

        // Touch pad events
        TOUCHPAD_CLICK(5),
        TOUCHPAD_TOUCH(6),

        // Orientation Events
        CONTROLLER_GAZE(7),
        CONTROLLER_ROTATE(8),
        CONTROLLER_MOVEMENT(9),
        CONTROLLER_STATUS(10);

        public final int mTypeId;
        EventSource(int id){
            mTypeId = id;
        }

        private static Map<Integer, EventSource> map = new HashMap<Integer, EventSource>();
        static {
            for (EventSource source : EventSource.values()) {
                map.put(source.mTypeId, source);
            }
        }
        public static EventSource valueOf(int id) {
            return map.get(id);
        }
    }

    /**
     * EventAction types corresponding to VROEventDelegate.h, used for
     * describing EventSource types. For example, if a click event
     * was CLICK_UP or CLICK_DOWN.
     *
     * IMPORTANT: Do Not change the Enum Values!!! Simply add additional
     * event types as need be. This should always map directly to
     * VROEventDelegate.h
     */
    public enum EventAction {
        CLICK_UP(1),
        CLICK_DOWN(2),
        GAZE_ON(3),
        GAZE_OFF(4),
        TOUCH_ON(5),
        TOUCH_OFF(6);

        public final int mTypeId;

        EventAction(int id){
            mTypeId = id;
        }

        private static Map<Integer, EventAction> map = new HashMap<Integer, EventAction>();
        static {
            for (EventAction action : EventAction.values()) {
                map.put(action.mTypeId, action);
            }
        }
        public static EventAction valueOf(int id) {
            return map.get(id);
        }
    }

    /**
     * ControllerStatus types describing the availability status of the
     * current input controller.
     *
     * IMPORTANT: Do Not change the Enum Values!!! Simply add additional
     * event types as need be. This should always map directly to
     * VROEventDelegate.h
     */
    public enum ControllerStatus {
        UNKNOWN(1),
        CONNECTING(2),
        CONNECTED(3),
        DISCONNECTED(4),
        ERROR(5);

        public final int mTypeId;

        ControllerStatus(int id){
            mTypeId = id;
        }

        private static Map<Integer, ControllerStatus> map = new HashMap<Integer, ControllerStatus>();
        static {
            for (ControllerStatus status : ControllerStatus.values()) {
                map.put(status.mTypeId, status);
            }
        }
        public static ControllerStatus valueOf(int id) {
            return map.get(id);
        }
    }

    /**
     * Delegate interface to be implemented by a java view component so as
     * to receive event callbacks triggered from native. Implemented delegates
     * must be set within EventDelegateJni through setEventDelegateCallback()
     * for it to be triggered.
     *
     * These callbacks correspond to the set or subset of EventTypes above
     * (Note they may not correspond 1 to 1 - certain EvenTypes may not yet
     * be needed or useful for Java views or components).
     */
    public interface EventDelegateCallback {
        void onControllerStatus(ControllerStatus status);
        void onButtonEvent(EventSource type, EventAction event);
        void onTouchpadEvent(EventSource type, EventAction event, float x, float y);
        void onRotate(float x, float y , float z);
        void onPosition(float x, float y , float z);
        void onGaze(boolean isGazing);
    }

    /**
     * Callback functions called from JNI (triggered from native)
     * that then triggers the corresponding EventDelegateCallback (mDelegate)
     * that has been set through setEventDelegateCallback().
     */
    void onControllerStatus(int status){
        if (mDelegate != null){
            mDelegate.onControllerStatus(ControllerStatus.valueOf(status));
        }
    }
    void onButtonEvent(int source, int action){
        if (mDelegate != null){
            mDelegate.onButtonEvent(EventSource.valueOf(source), EventAction.valueOf(action));
        }
    }
    void onTouchpadEvent(int source, int action, float x, float y){
        if (mDelegate != null){
            mDelegate.onTouchpadEvent(EventSource.valueOf(source),
                    EventAction.valueOf(action), x,y);
        }
    }
    void onRotate(float x, float y , float z){
        if (mDelegate != null){
            mDelegate.onRotate(x,y,z);
        }
    }
    void onPosition(float x, float y , float z){
        if (mDelegate != null){
            mDelegate.onPosition(x,y,z);
        }
    }
    void onGaze(boolean isGazing){
        if (mDelegate != null){
            mDelegate.onGaze(isGazing);
        }
    }
    void onGazeHitDistance(float distance){
        //No-op
    }
}
