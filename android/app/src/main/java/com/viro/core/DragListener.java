/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import com.viro.core.Node;
import com.viro.core.Vector;

/**
 * Callback interface for responding to drag events, which occur when the Controller pressed
 * down on a {@link Node} with a button, holds it, and then drags the {@link Node}.
 */
public interface DragListener {

    /**
     * Callback when a drag motion (click down, then move) is detected for the given {@link Node}.
     *
     * @param source        The platform specific source ID, which indicates what button or
     *                      component on the Controller triggered the event. See the Controller's
     *                      Guide for information.
     * @param node          The {@link Node} that is being dragged.
     * @param worldLocation The location in world coordinates to which the {@link Node} is being
     *                      dragged.
     * @param localLocation The location in the coordinate system of the Node's parent to which the
     *                      {@link Node} is being dragged.
     */
    void onDrag(int source, Node node, Vector worldLocation, Vector localLocation);
}