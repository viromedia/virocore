/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Java JNI Wrapper     : com.viro.renderer.RenderContextJni.java
 * Cpp JNI wrapper      : RenderContext_JNI.cpp
 * Cpp Object           : RenderContext
 */
public class RenderContext {
    protected long mNativeRef;

    public RenderContext(long mRenderRef){
        mNativeRef = nativeCreateRenderContext(mRenderRef);
    }

    public void delete(){
        nativeDeleteRenderContext(mNativeRef);
    }

    public void getCameraOrientation(CameraCallback callback) {
        nativeGetCameraOrientation(mNativeRef, callback);
    }

    private native long nativeCreateRenderContext(long mNativeRenderer);
    private native void nativeDeleteRenderContext(long mNativeContextRef);
    private native void nativeGetCameraOrientation(long mNativecontextRef, CameraCallback callback);
}