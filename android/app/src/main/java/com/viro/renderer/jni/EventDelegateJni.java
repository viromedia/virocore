/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

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
    public void setEventEnabled(EventType type, boolean enabled) {
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
     * EventTypes corresponding to VROEventDelegate.h, used for enabling or
     * disabling delegate event callbacks through the JNI layer.
     *
     * EventTypes used in Java are mapped to the function callbacks within
     * EventDelegateCallbacks.
     *
     * IMPORTANT: Do Not change the Enum Values!!! Simply add additional
     * event types as need be. This should always map directly to
     * VROEventDelegate.h
     */
    public enum EventType {
        ON_TAP(1),
        ON_GAZE(2),
        ON_GAZE_DISTANCE(3);

        public final int mTypeId;

        EventType(int id){
            mTypeId = id;
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
        void onTapped();
        void onGaze(boolean isGazing);
    }

    /**
     * Callback functions called from JNI (triggered from native)
     * that then triggers the corresponding EventDelegateCallback (mDelegate)
     * that has been set through setEventDelegateCallback().
     */
    public void onTapped() {
        if (mDelegate != null){
            mDelegate.onTapped();
        }
    }

    public void onGaze(boolean isGazing) {
        if (mDelegate != null){
            mDelegate.onGaze(isGazing);
        }
    }
}
