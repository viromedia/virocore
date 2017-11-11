/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.Controller.java
 * Java JNI Wrapper     : com.viro.renderer.ControllerJni.java
 * Cpp JNI wrapper      : Controller_JNI.cpp
 * Cpp Object           : VROInputPresenter
 */
package com.viro.core;

/**
 * Controller represents the UI through which the user interacts with the {@link Scene}. The exact
 * form of Controller depends on the underlying platform. For example, for Daydream this represents
 * the Daydream controller (the laser pointer). For Cardboard and GearVR, Controller is effectively
 * the head-mounted display itself (and its tap button).
 * <p>
 * Controllers are also able to listen to all events. The Controller is is notified of all events
 * that occur within the Scene, regardless of what {@link Node} those events are associated with
 * (with the exception of Hover events). This is useful if you wish to be alerted of all events
 * occurring in the Scene in a centralized place.
 * <p>
 * Controller should not be constructed by your app. It is retrieved via {@link
 * ViroView#getController()}.
 */
public class Controller implements EventDelegate.EventDelegateCallback {

    private ViroContext mViroContext;
    private boolean mReticleVisible = true;
    private boolean mControllerVisible = true;

    private EventDelegate mEventDelegate;
    private ClickListener mClickListener;
    private HoverListener mHoverListener;
    private ControllerStatusListener mStatusListener;
    private TouchpadTouchListener mTouchpadTouchListener;
    private TouchpadSwipeListener mTouchpadSwipeListener;
    private TouchpadScrollListener mTouchpadScrollListener;
    private DragListener mDragListener;
    private FuseListener mFuseListener;
    private GesturePinchListener mGesturePinchListener;
    private GestureRotateListener mGestureRotateListener;

    /**
     * @hide
     * @param viroContext
     */
    Controller(ViroContext viroContext) {
        mViroContext = viroContext;
        mEventDelegate = new EventDelegate();
        mEventDelegate.setEventDelegateCallback(this);
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
     * Release native resources associated with this Controller.
     */
    public void dispose() {
        mEventDelegate.dispose();
    }

    /**
     * @hide
     * @param delegate
     */
    public void setEventDelegate(EventDelegate delegate) {
        if (mEventDelegate != null) {
            mEventDelegate.dispose();
        }
        mEventDelegate = delegate;
        nativeSetEventDelegate(mViroContext.mNativeRef, delegate.mNativeRef);
    }

    /**
     * Set the reticle visibility on or off. The reticle is the small pointer that appears at the
     * center of the screen, which is useful for tapping and fusing on objects.
     * <p>
     * Defaults to true.
     *
     * @param visible True to make the reticle visible, false to make it invisible.
     */
    public void setReticleVisible(boolean visible) {
        mReticleVisible = visible;
        nativeEnableReticle(mViroContext.mNativeRef, visible);
    }

    /**
     * Returns true if the reticle is currently visible.
     *
     * @return True if the reticle is visible.
     */
    public boolean isReticleVisible() {
        return mReticleVisible;
    }

    /**
     * Set the controller to visible or invisible. This currently only applies to the Daydream
     * platform, and determines whether the Daydream controller is displayed on the screen.
     * <p>
     * Defaults to true.
     *
     * @param visible True to display the controller.
     */
    public void setControllerVisible(boolean visible) {
        mControllerVisible = visible;
        nativeEnableController(mViroContext.mNativeRef, visible);
    }

    /**
     * Returns true if the Controller is currently visible.
     *
     * @return True if the Controller is visible.
     */
    public boolean isControllerVisible() {
        return mControllerVisible;
    }

    /**
     * Get the direction in which the Controller is pointing. For Daydream devices this returns
     * the direction the pointer if pointing, and for other devices it returns the direction the
     * user is facing (e.g. in the direction of the reticle).
     *
     * @return The direction the Controller is pointing.
     */
    public Vector getForwardVector() {
        return new Vector(nativeGetControllerForwardVector(mViroContext.mNativeRef));
    }

    /**
     * @hide
     * @param callback
     */
    public void getControllerForwardVectorAsync(ControllerJniCallback callback){
        nativeGetControllerForwardVectorAsync(mViroContext.mNativeRef, callback);
    }

    /**
     * Set a {@link ClickListener} to respond when users click with the Controller.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setClickListener(ClickListener listener) {
        mClickListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_CLICK, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_CLICK, false);
        }
    }

    /**
     * Get the {@link ClickListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public ClickListener getClickListener() {
        return mClickListener;
    }

    /**
     * Set the {@link TouchpadTouchListener} to respond when a user touches or moves across
     * a touchpad Controller.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setTouchpadTouchListener(TouchpadTouchListener listener) {
        mTouchpadTouchListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_TOUCH, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_TOUCH, false);
        }
    }

    /**
     * Get the {@link TouchpadTouchListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public TouchpadTouchListener getTouchpadTouchListener() {
        return mTouchpadTouchListener;
    }

    /**
     * Set the {@link TouchpadSwipeListener} to respond when a user swipes across a touchpad
     * Controller.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setTouchpadSwipeListener(TouchpadSwipeListener listener) {
        mTouchpadSwipeListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_SWIPE, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_SWIPE, false);
        }
    }

    /**
     * Get the {@link TouchpadSwipeListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public TouchpadSwipeListener getTouchpadSwipeListener() {
        return mTouchpadSwipeListener;
    }

    /**
     * Set the {@link TouchpadScrollListener} to respond when a user scrolls a touchpad.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setTouchpadScrollListener(TouchpadScrollListener listener) {
        mTouchpadScrollListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_SCROLL, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_SCROLL, false);
        }
    }

    /**
     * Get the {@link TouchpadScrollListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public TouchpadScrollListener getTouchpadScrollListener() {
        return mTouchpadScrollListener;
    }

    /**
     * Set the {@link DragListener} to respond when a user attempts to drag a {@link Node}.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setDragListener(DragListener listener) {
        mDragListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_DRAG, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_DRAG, false);
        }
    }

    /**
     * Get the {@link DragListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public DragListener getDragListener() {
        return mDragListener;
    }

    /**
     * Set the {@link FuseListener} to respond when a user hovers over a {@link Node}
     * long enough to cause a fuse.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setFuseListener(FuseListener listener) {
        mFuseListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_FUSE, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_FUSE, false);
        }
    }

    /**
     * Get the {@link FuseListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public FuseListener getFuseListener() {
        return mFuseListener;
    }

    /**
     * Set the {@link GesturePinchListener} to respond when a user pinches with two fingers over
     * a {@link Node} using a screen Controller.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setGesturePinchListener(GesturePinchListener listener) {
        mGesturePinchListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_PINCH, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_PINCH, false);
        }
    }

    /**
     * Get the {@link GesturePinchListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public GesturePinchListener getGesturePinchListener() {
        return mGesturePinchListener;
    }

    /**
     * Set the {@link GestureRotateListener} to respond when a user rotates with two fingers over
     * a {@link Node} using a screen Controller.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setGestureRotateListener(GestureRotateListener listener) {
        mGestureRotateListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_ROTATE, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_ROTATE, false);
        }
    }

    /**
     * Get the {@link GestureRotateListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public GestureRotateListener getGestureRotateListener() {
        return mGestureRotateListener;
    }

    /**
     * Set the {@link ControllerStatusListener} to respond when a Controller connects, disconnects,
     * or enters an error state.
     *
     * @param listener The listener to attach, or null to remove any installed listener.
     */
    public void setControllerStatusListener(ControllerStatusListener listener) {
        mStatusListener = listener;
        if (listener != null) {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_CONTROLLER_STATUS, true);
        }
        else {
            mEventDelegate.setEventEnabled(EventDelegate.EventAction.ON_CONTROLLER_STATUS, false);
        }
    }

    /**
     * Get the {@link ControllerStatusListener} that is currently installed for this Controller.
     *
     * @return The installed listener, or null if none is installed.
     */
    public ControllerStatusListener getControllerStatusListener() {
        return mStatusListener;
    }

    /**
     * This is used for real-time depth testing, only by the VRTScene. Not exposed to Java API.
     * @hide
     */
    @Override
    public void onCameraARHitTest(int source, ARHitTestResult[] results) {

    }

    /**
     * @hide
     */
    @Override
    public void onClick(int source, Node node, ClickState clickState, float[] hitLoc) {
        if (mClickListener != null) {
            mClickListener.onClickState(source, node, clickState, new Vector(hitLoc));
            if (clickState == ClickState.CLICKED) {
                mClickListener.onClick(source, node, new Vector(hitLoc));
            }
        }
    }
    /**
     * @hide
     */
    @Override
    public void onHover(int source, Node node, boolean isHovering, float[] hitLoc) {
        if (mHoverListener != null) {
            mHoverListener.onHover(source, node, isHovering, new Vector(hitLoc));
        }
    }
    /**
     * @hide
     */
    @Override
    public void onControllerStatus(int source, ControllerStatus status) {
        if (mStatusListener != null) {
            mStatusListener.onControllerStatus(source, status);
        }
    }
    /**
     * @hide
     */
    @Override
    public void onTouch(int source, Node node, TouchState touchState, float[] touchPadPos) {
        if (mTouchpadTouchListener != null) {
            mTouchpadTouchListener.onTouch(source, node, touchState, touchPadPos[0], touchPadPos[1]);
        }
    }
    /**
     * @hide
     */
    @Override
    public void onScroll(int source, Node node, float x, float y) {
        if (mTouchpadScrollListener != null) {
            mTouchpadScrollListener.onScroll(source, node, x, y);
        }
    }
    /**
     * @hide
     */
    @Override
    public void onSwipe(int source, Node node, SwipeState swipeState) {
        if (mTouchpadSwipeListener != null) {
            mTouchpadSwipeListener.onSwipe(source, node, swipeState);
        }
    }
    /**
     * @hide
     */
    @Override
    public void onDrag(int source, Node node, float x, float y, float z) {
        if (mDragListener != null) {
            Vector local = new Vector(x, y, z);

            // We have to convert the local drag coordinates to world space
            Vector world = new Vector(x, y, z);
            if (node != null && node.getParentNode() != null) {
                world = node.getParentNode().convertLocalPositionToWorldSpace(local);
            }
            mDragListener.onDrag(source, node, world, local);
        }
    }
    /**
     * @hide
     */
    @Override
    public void onFuse(int source, Node node) {
        if (mFuseListener != null) {
            mFuseListener.onFuse(source, node);
        }
    }
    /**
     * @hide
     */
    @Override
    public void onPinch(int source, Node node, float scaleFactor, PinchState pinchState) {
        if (mGesturePinchListener != null) {
            mGesturePinchListener.onPinch(source, node, scaleFactor, pinchState);
        }
    }
    /**
     * @hide
     */
    @Override
    public void onRotate(int source, Node node, float rotationRadians, RotateState rotateState) {
        if (mGestureRotateListener != null) {
            mGestureRotateListener.onRotate(source, node, rotationRadians, rotateState);
        }
    }

    private native void nativeSetEventDelegate(long contextRef, long delegateRef);
    private native void nativeEnableReticle(long contextRef, boolean enabled);
    private native void nativeEnableController(long contextRef, boolean enabled);
    private native float[] nativeGetControllerForwardVector(long contextRef);
    private native void nativeGetControllerForwardVectorAsync(long renderContextRef,
                                                              ControllerJniCallback callback);

    /**
     * @hide
     */
    public interface ControllerJniCallback{
        void onGetForwardVector(float x, float y, float z);
    }
}
