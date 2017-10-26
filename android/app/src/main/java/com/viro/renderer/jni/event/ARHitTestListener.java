package com.viro.renderer.jni.event;

import com.viro.renderer.ARHitTestResult;
import com.viro.renderer.jni.Node;

/**
 * Callback interface for responding to AR hit test events, which occur after an AR hit test
 * intersects a {@link Node}. TODO DOC Indicate how AR hit tests are triggered
 */
public interface ARHitTestListener {

    /**
     * TODO DOC
     *
     * @param source
     * @param results
     */
    void onARHitTest(int source, ARHitTestResult[] results);
}
