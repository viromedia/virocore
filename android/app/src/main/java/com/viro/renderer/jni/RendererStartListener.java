/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Callback interface for responding to the Viro renderer finishing initialization. This listener
 * are used by the subclasses of {@link ViroView}.
 */
public interface RendererStartListener {

    /**
     * Callback invoked when {@link ViroView} has finished initialization. When this is received,
     * the ViroView is ready to begin rendering content.
     */
    void onRendererStart();
}
