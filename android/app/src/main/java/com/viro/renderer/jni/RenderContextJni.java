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
public class RenderContextJni {
    protected long mRef;

    public RenderContextJni(long mRenderRef){
        mRef = nativeCreateRenderContext(mRenderRef);
    }

    public void delete(){
        nativeDeleteRenderContext(mRef);
    }

    public float[] getCameraPosition() { return nativeGetCameraPosition(mRef); }

    private native long nativeCreateRenderContext(long mNativeRenderer);
    private native void nativeDeleteRenderContext(long mNativeContextRef);
    private native float[] nativeGetCameraPosition(long mNativecontextRef);
}
