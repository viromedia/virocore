/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
/*
 * Java JNI wrapper responsible for registering and implementing event
 * delegate callbacks across the JNI bridge. It ultimately links the
 * following classes below.
 *
 * Java JNI Wrapper     : com.viro.renderer.EventDelegateJni.java
 * Cpp JNI wrapper      : EventDelegate_JNI.cpp
 * Cpp Object           : VROEventDelegate.cpp
 */
package com.viro.core;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

/**
 * @hide
 */
public class EventDelegate {

    /*
     * Delegate interface to be implemented by a java view component so as
     * to receive event callbacks triggered from native. Implemented delegates
     * must be set within EventDelegateJni through setEventDelegateCallback()
     * for it to be triggered.
     *
     * These callbacks correspond to the set or subset of EventTypes above
     * (Note they may not correspond 1 to 1 - certain EvenTypes may not yet
     * be needed or useful for Java views or components).
     *
     * @hide
     */
    public interface EventDelegateCallback {
        /**
         * @hide
         */
        void onHover(int source, Node node, boolean isHovering, float hitLoc[]);
        /**
         * @hide
         */
        void onClick(int source, Node node, ClickState clickState, float hitLoc[]);
        /**
         * @hide
         */
        void onTouch(int source, Node node, TouchState touchState, float touchPadPos[]);
        /**
         * @hide
         */
        void onControllerStatus(int source, ControllerStatus status);
        /**
         * @hide
         */
        void onSwipe(int source, Node node, SwipeState swipeState);
        /**
         * @hide
         */
        void onScroll(int source, Node node, float x, float y);
        /**
         * @hide
         */
        void onDrag(int source, Node node, float x, float y, float z);
        /**
         * @hide
         */
        void onFuse(int source, Node node);
        /**
         * @hide
         */
        void onPinch(int source, Node node, float scaleFactor, PinchState pinchState);
        /**
         * @hide
         */
        void onRotate(int source, Node node, float rotationRadians, RotateState rotateState);
        /**
         * @hide
         */
        void onCameraARHitTest(ARHitTestResult[] results);
        /**
         * @hide
         */
        void onARPointCloudUpdate(ARPointCloud pointCloud);
        /**
         * @hide
         */
        void onCameraTransformUpdate(float posX, float poxY, float posZ,
                                     float rotEulerX, float rotEulerY, float rotEulerZ,
                                     float forwardX, float forwardY, float forwardZ,
                                     float upX, float upY, float upZ);
    }

    long mNativeRef;
    private float mTimeToFuse;
    private WeakReference<EventDelegateCallback> mDelegate = null;

    /**
     * Construct a new EventDelegate.
     */
    public EventDelegate() {
        mNativeRef = nativeCreateDelegate();
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this EventDelegate.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyDelegate(mNativeRef);
            mNativeRef = 0;
        }
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
        mDelegate = new WeakReference<EventDelegateCallback>(delegate);
    }

    public void setTimeToFuse(float durationInMillis) {
        mTimeToFuse = durationInMillis;
        nativeSetTimeToFuse(mNativeRef, durationInMillis);
    }

    public float getTimeToFuse() {
        return mTimeToFuse;
    }

    /*
     Native Functions called into JNI
     */
    private native long nativeCreateDelegate();
    private native void nativeDestroyDelegate(long mNativeNodeRef);
    private native void nativeEnableEvent(long mNativeNodeRef, int eventType, boolean enabled);
    private native void nativeSetTimeToFuse(long mNativeNodeRef, float durationInMillis);

    /*
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
        ON_CAMERA_AR_HIT_TEST(12),
        ON_AR_POINT_CLOUD_UPDATE(13),
        ON_CAMERA_TRANSFORM_UPDATE(14);

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

    /*
     Callback functions called from JNI (triggered from native)
     that then trigger the corresponding EventDelegateCallback (mDelegate)
     that has been set through setEventDelegateCallback().
     */

    /**
     * @hide
     */
    void onHover(int source, int nodeId, boolean isHovering, float[] position) {
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onHover(source, node, isHovering, position);
        }
    }
    /**
     * @hide
     */
    void onClick(int source, int nodeId, int clickState, float[] position) {
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onClick(source, node, ClickState.valueOf(clickState), position);
        }
    }
    /**
     * @hide
     */
    void onControllerStatus(int source, int status) {
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onControllerStatus(source, ControllerStatus.valueOf(status));
        }
    }
    /**
     * @hide
     */
    void onTouch(int source, int nodeId, int touchState, float x, float y){
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onTouch(source, node, TouchState.valueOf(touchState), new float[]{x,y});
        }
    }
    /**
     * @hide
     */
    void onSwipe(int source, int nodeId, int swipeState){
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onSwipe(source, node, SwipeState.valueOf(swipeState));
        }
    }
    /**
     * @hide
     */
    void onScroll(int source, int nodeId, float x, float y){
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onScroll(source, node, x, y);
        }
    }
    /**
     * @hide
     */
    void onDrag(int source, int nodeId, float x, float y, float z){
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onDrag(source, node, x, y, z);
        }
    }
    /**
     * @hide
     */
    void onFuse(int source, int nodeId) {
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onFuse(source, node);
        }
    }
    /**
     * @hide
     */
    void onPinch(int source, int nodeId, float scaleFactor, int pinchState) {
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null)  {
            mDelegate.get().onPinch(source, node, scaleFactor, PinchState.valueOf(pinchState));
        }
    }
    /**
     * @hide
     */
    void onRotate(int source, int nodeId, float rotationRadians, int rotationState) {
        Node node = Node.getNodeWithID(nodeId);
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onRotate(source, node, rotationRadians, RotateState.valueOf(rotationState));
        }
    }

    void onCameraARHitTest(ARHitTestResult[] results) {
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onCameraARHitTest(results);
        }
    }

    void onARPointCloudUpdate(ARPointCloud pointCloud) {
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onARPointCloudUpdate(pointCloud);
        }
    }

    /**
     * @hide
     */
    void onCameraTransformUpdate(float posX, float poxY, float posZ,
                                 float rotEulerX, float rotEulerY, float rotEulerZ,
                                 float forwardX, float forwardY, float forwardZ,
                                 float upX, float upY, float upZ) {
        if (mDelegate != null && mDelegate.get() != null) {
            mDelegate.get().onCameraTransformUpdate(posX, poxY, posZ, rotEulerX, rotEulerY, rotEulerZ,
                                                    forwardX, forwardY, forwardZ, upX, upY, upZ);
        }
    }
}
