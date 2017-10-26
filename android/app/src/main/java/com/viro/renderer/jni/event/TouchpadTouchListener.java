package com.viro.renderer.jni.event;

import com.viro.renderer.jni.Node;

/**
 * Callback interface for responding to touchpad touch events, which occur when a finger
 * interacts with a touchpad Controller.
 */
public interface TouchpadTouchListener {

    /**
     * Callback when a touchpad touch event is received, meaning the user has placed and/or
     * moved a finger on a touchpad controller while hovering over the {@link Node} receiving
     * this event.
     *
     * @param source     The platform specific source ID, which indicates what button or
     *                   component on the Controller triggered the event. See the Controller's
     *                   Guide for information.
     * @param node       The {@link Node} being hovered over when the touch is received.
     * @param touchState The state of the touch; e.g. down, up, moving.
     * @param x          The X coordinate on the touchpad where the event occurred.
     * @param y          The Y coordinate on the touchpad where the event occurred.
     */
    void onTouch(int source, Node node, TouchState touchState, float x, float y);
}
