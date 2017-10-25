/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import java.util.HashMap;
import java.util.Map;
import com.viro.renderer.ARHitTestResult;

/**
 * Java JNI wrapper responsible for registering and implementing event
 * delegate callbacks across the JNI bridge. It ultimately links the
 * following classes below.
 *
 * Java JNI Wrapper     : com.viro.renderer.EventDelegateJni.java
 * Cpp JNI wrapper      : EventDelegate_JNI.cpp
 * Cpp Object           : VROEventDelegate.cpp
 */
public class EventDelegate {
    final protected long mNativeRef;
    private EventDelegateCallback mDelegate = null;

    public EventDelegate() {
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
    public void setEventEnabled(EventAction type, boolean enabled) {
        nativeEnableEvent(mNativeRef, type.mTypeId, enabled);
    }

    public void setEventDelegateCallback(EventDelegateCallback delegate) {
        mDelegate = delegate;
    }
    public void setTimeToFuse(float durationInMillis){
        nativeSetTimeToFuse(mNativeRef, durationInMillis);
    }

    /**
     * Native Functions called into JNI
     */
    private native long nativeCreateDelegate();
    private native void nativeDestroyDelegate(long mNativeNodeRef);
    private native void nativeEnableEvent(long mNativeNodeRef, int eventType, boolean enabled);
    private native void nativeSetTimeToFuse(long mNativeNodeRef, float durationInMillis);
    /**
     * EventAction types corresponding to VROEventDelegate.h, used for
     * describing EventSource types. For example, if a click event
     * was ClickUp or ClickDown.
     *
     * IMPORTANT: Do Not change the Enum Values!!! Simply add additional
     * event types as need be. This should always map directly to
     * VROEventDelegate.h
     */
    public enum EventAction {
        ON_HOVER(1),
        ON_CLICK(2),
        ON_TOUCH(3),
        ON_MOVE(4),
        ON_CONTROLLER_STATUS(5),
        ON_SWIPE(6),
        ON_SCROLL(7),
        ON_DRAG(8),
        ON_FUSE(9),
        ON_PINCH(10),
        ON_ROTATE(11),
        ON_CAMERA_AR_HIT_TEST(12);

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

    public enum ClickState {
        CLICK_DOWN(1),
        CLICK_UP(2),
        CLICKED(3);

        public final int mTypeId;

        ClickState(int id){
            mTypeId = id;
        }

        private static Map<Integer, ClickState> map = new HashMap<Integer, ClickState>();
        static {
            for (ClickState status : ClickState.values()) {
                map.put(status.mTypeId, status);
            }
        }
        public static ClickState valueOf(int id) {
            return map.get(id);
        }
    }

    public enum TouchState {
        TOUCH_DOWN(1),
        TOUCH_DOWN_MOVE(2),
        TOUCH_UP(3);

        public final int mTypeId;

        TouchState(int id){
            mTypeId = id;
        }

        private static Map<Integer, TouchState> map = new HashMap<Integer, TouchState>();
        static {
            for (TouchState status : TouchState.values()) {
                map.put(status.mTypeId, status);
            }
        }
        public static TouchState valueOf(int id) {
            return map.get(id);
        }
    }

    public enum SwipeState {
        SWIPE_UP(1),
        SWIPE_DOWN(2),
        SWIPE_LEFT(3),
        SWIPE_RIGHT(4);

        public final int mTypeId;

        SwipeState(int id){
            mTypeId = id;
        }

        private static Map<Integer, SwipeState> map = new HashMap<Integer, SwipeState>();
        static {
            for (SwipeState status : SwipeState.values()) {
                map.put(status.mTypeId, status);
            }
        }
        public static SwipeState valueOf(int id) {
            return map.get(id);
        }
    }

    public enum PinchState {
        PINCH_START(1),
        PINCH_MOVE(2),
        PINCH_END(3);

        public final int mTypeId;

        PinchState(int id) {
            mTypeId = id;
        }

        private static Map<Integer, PinchState> map = new HashMap<Integer, PinchState>();
        static {
            for (PinchState status : PinchState.values()) {
                map.put(status.mTypeId, status);
            }
        }
        public static PinchState valueOf(int id) {
            return map.get(id);
        }
    }


    public enum RotateState {
        ROTATE_START(1),
        ROTATE_MOVE(2),
        ROTATE_END(3);

        public final int mTypeId;

        RotateState(int id) {
            mTypeId = id;
        }

        private static Map<Integer, RotateState> map = new HashMap<Integer, RotateState>();
        static {
            for (RotateState status : RotateState.values()) {
                map.put(status.mTypeId, status);
            }
        }
        public static RotateState valueOf(int id) {
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
        void onHover(int source, boolean isHovering, float hitLoc[]);
        void onClick(int source, ClickState clickState, float hitLoc[]);
        void onTouch(int source, TouchState touchState, float touchPadPos[]);
        void onControllerStatus(int source, ControllerStatus status);
        void onSwipe(int source, SwipeState swipeState);
        void onScroll(int source, float x, float y);
        void onDrag(int source, float x, float y, float z);
        void onFuse(int source);
        void onPinch(int source, float scaleFactor, PinchState pinchState);
        void onRotate(int source, float rotateFactor, RotateState rotateState);
        void onCameraARHitTest(int source, ARHitTestResult[] results);
    }

    /**
     * Callback functions called from JNI (triggered from native)
     * that then triggers the corresponding EventDelegateCallback (mDelegate)
     * that has been set through setEventDelegateCallback().
     */
    void onHover(int source, boolean isHovering, float[] position) {
        if (mDelegate != null){
            mDelegate.onHover(source, isHovering, position);
        }
    }

    void onClick(int source, int clickState, float[] position) {
        if (mDelegate != null){
            mDelegate.onClick(source, ClickState.valueOf(clickState), position);
        }
    }

    void onControllerStatus(int source, int status) {
        if (mDelegate != null){
            mDelegate.onControllerStatus(source, ControllerStatus.valueOf(status));
        }
    }

    void onTouch(int source, int touchState, float x, float y){
        if (mDelegate != null){
            mDelegate.onTouch(source, TouchState.valueOf(touchState), new float[]{x,y});
        }
    }

    void onSwipe(int source, int swipeState){
        if (mDelegate != null){
            mDelegate.onSwipe(source, SwipeState.valueOf(swipeState));
        }
    }

    void onScroll(int source, float x, float y){
        if (mDelegate != null){
            mDelegate.onScroll(source, x, y);
        }
    }

    void onDrag(int source, float x, float y, float z){
        if (mDelegate != null){
            mDelegate.onDrag(source, x, y, z);
        }
    }

    void onFuse(int source) {
        if (mDelegate != null){
            mDelegate.onFuse(source);
        }
    }

    void onPinch(int source, float scaleFactor, int pinchState) {
        if (mDelegate != null) {
            mDelegate.onPinch(source, scaleFactor, PinchState.valueOf(pinchState));
        }
    }

    void onRotate(int source, float rotationDegrees, int rotationState) {
        if (mDelegate != null) {
            mDelegate.onRotate(source, rotationDegrees, RotateState.valueOf(rotationState));
        }
    }

    void onCameraARHitTest(int source, ARHitTestResult[] results) {
        if(mDelegate != null) {
            mDelegate.onCameraARHitTest(source, results);
        }
    }
}
