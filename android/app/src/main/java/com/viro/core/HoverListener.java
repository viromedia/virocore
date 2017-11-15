/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import com.viro.core.Node;
import com.viro.core.Vector;

/**
 * Callback interface for responding to hover events, which occur when the Controller hovers
 * over a {@link Node}.
 */
public interface HoverListener {

    /**
     * Callback when a hover event is received; that is, when the Controller begins or ends
     * hovering over the given {@link Node}.
     *
     * @param source     The platform specific source ID, which indicates what button or
     *                   component on the Controller triggered the event. See the Controller's
     *                   Guide for information.
     * @param node       The {@link Node} which gained or lost hover.
     * @param isHovering True if the Controller is hovering over this {@link Node}, false
     *                   otherwise.
     * @param location   The location of the event in world coordinates.
     */
    void onHover(int source, Node node, boolean isHovering, Vector location);
}
