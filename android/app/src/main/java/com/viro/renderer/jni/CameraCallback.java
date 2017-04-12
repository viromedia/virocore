 /**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Callback interface used to get camera related information from the
 * renderer asynchronously.
 */
public interface CameraCallback {
    void onGetCameraPosition(float x, float y, float z);
}
