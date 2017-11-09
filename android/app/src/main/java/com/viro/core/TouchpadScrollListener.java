/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.core;

import com.viro.core.Node;

/**
 * Callback interface for responding to touchpad scroll events, which occur when the finger is
 * used to scroll up or down while hovering over a {@link Node} in a touchpad Controller.
 */
public interface TouchpadScrollListener {

    /**
     * Callback when a scrolling gesture is detected on the touchpad while hovering over
     * the given {@link Node}.
     *
     * @param source The platform specific source ID, which indicates what button or component
     *               on the Controller triggered the event. See the Controller's Guide for
     *               information.
     * @param node   The {@link Node} being hovered over while the scroll gesture is taking
     *               place.
     * @param x      The X coordinate on the touchpad where the event occurred.
     * @param y      The Y coordinate on the touchpad where the event occurred.
     */
    void onScroll(int source, Node node, float x, float y);
}
