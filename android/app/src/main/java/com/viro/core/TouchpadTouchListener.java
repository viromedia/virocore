/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.core;

/**
 * Callback interface for responding to touchpad touch events, which occur when a finger
 * interacts with a touchpad Controller.
 */
public interface TouchpadTouchListener {

    /**
     * Callback when a touchpad touch event is received, meaning the user has placed and/or
     * moved a finger on a touchpad controller while hovering over the given {@link Node}.
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
