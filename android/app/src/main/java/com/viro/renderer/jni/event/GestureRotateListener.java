package com.viro.renderer.jni.event;

import com.viro.renderer.jni.Node;

/**
 * Callback interface for responding to rotate events, which occur when the user places two
 * fingers on the screen and rotates then clockwise or counterclockwise.
 */
public interface GestureRotateListener {

    /**
     * Callback when a two-finger rotate gesture is detected on the Controller screen. This is
     * used in AR.
     *
     * @param source        The platform specific source ID, which indicates what button or
     *                      component on the Controller triggered the event. See the
     *                      Controller's Guide for information.
     * @param node          The {@link Node} on which the gesture is being performed.
     * @param rotateDegrees The degrees of rotation of the gesture since the beginning of the
     *                      event; specifically, since receiving the ROTATE_START state.
     * @param rotateState   The state of the rotate event.
     */
    void onRotate(int source, Node node, float rotateDegrees, RotateState rotateState);
}