/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public interface AsyncObjListener {
    /**
     * Invoked on main thread after the OBJ is loaded.
     */
    void onObjLoaded();

    /**
     * Invoked on the rendering thread after the OBJ is added to the scene.
     */
    void onObjAttached();

    /**
     * Invoked on the main thread if the OBJ fails to load.
     */
    void onObjFailed(String error);
}
