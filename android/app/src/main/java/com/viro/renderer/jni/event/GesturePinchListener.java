package com.viro.renderer.jni.event;

import com.viro.renderer.jni.Node;

/**
 * Callback interface for responding to pinch events, which occur when the user pinches with two
 * fingers on the screen.
 */
public interface GesturePinchListener {

    /**
     * Callback when a two-finger pinch gesture is detected on the Controller screen. This is
     * used in AR.
     *
     * @param source      The platform specific source ID, which indicates what button or
     *                    component on the Controller triggered the event. See the Controller's
     *                    Guide for information.
     * @param node        The {@link Node} on which the gesture is being performed.
     * @param scaleFactor The scale factor change since the beginning of the event;
     *                    specifically, since receiving the PINCH_START state. Scale starts at
     *                    1.0 and increases if pinches outward and decreases if pinched inward.
     * @param pinchState  The state of the pinch event.
     */
    void onPinch(int source, Node node, float scaleFactor, PinchState pinchState);
}