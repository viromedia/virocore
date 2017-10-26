package com.viro.renderer.jni.event;

import com.viro.renderer.jni.Node;

/**
 * Callback interface for responding to fuse events, which occur when the user hovers the
 * Controller's pointer over a {@link Node} for a set period of time.
 */
public interface FuseListener {

    /**
     * Callback when the {@link Node} receiving this event has fused, meaning the Controller
     * has hovered over this Node for <i>time to fuse</i> milliseconds. Time to fuse can be
     * set via {@link Node#setTimeToFuse(float)}.
     *
     * @param source The platform specific source ID, which indicates what button or component
     *               on the Controller triggered the event. See the Controller's Guide for
     *               information.
     * @param node   The {@link Node} being hovered over (the Node that is "fusing").
     */
    void onFuse(int source, Node node);
}