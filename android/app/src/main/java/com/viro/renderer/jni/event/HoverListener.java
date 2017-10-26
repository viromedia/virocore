package com.viro.renderer.jni.event;

import com.viro.renderer.jni.Node;
import com.viro.renderer.jni.Vector;

/**
 * Callback interface for responding to hover events, which occur when the Controller hovers
 * over a {@link Node}.
 */
public interface HoverListener {

    /**
     * Callback when a hover event is received; that is, when the Controller begins or ends
     * hovering over the {@link Node} receiving this event.
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
