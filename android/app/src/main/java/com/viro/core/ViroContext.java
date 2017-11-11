/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
/*
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Java JNI Wrapper     : com.viro.renderer.RenderContextJni.java
 * Cpp JNI wrapper      : RenderContext_JNI.cpp
 * Cpp Object           : RenderContext
 */
package com.viro.core;

import com.viro.core.internal.CameraCallback;

/**
 * ViroContext provides context for the Viro application. It is tied to a specific EGL Context
 * and is required for rendering tasks.
 */
public class ViroContext {

    long mNativeRef;
    private Controller mController;

    /**
     * Construct a new ViroContext with the native handle to the Renderer.
     *
     * @param mRenderRef The native peer.
     * @hide
     */
    ViroContext(long mRenderRef){
        mNativeRef = nativeCreateViroContext(mRenderRef);
        mController = new Controller(this);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this ViroContext.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDeleteViroContext(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreateViroContext(long mNativeRenderer);
    private native void nativeDeleteViroContext(long mNativeContextRef);
    private native void nativeGetCameraOrientation(long mNativeContextRef, CameraCallback callback);

    /**
     * @hide
     */
    public void getCameraOrientation(CameraCallback callback) {
        nativeGetCameraOrientation(mNativeRef, callback);
    }

    /**
     * @hide
     */
    public Controller getController() {
        return mController;
    }

}