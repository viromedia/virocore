/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.content.Context;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.Box.java
 * Java JNI Wrapper     : com.viro.renderer.BoxJni.java
 * Cpp JNI wrapper      : Box_JNI.cpp
 * Cpp Object           : VROBox.cpp
 */
public class BoxJni extends BaseGeometry{
    public BoxJni(long width, long height, long length) {
        mNativeRef = nativeCreateBox(
                getClass().getClassLoader(),
                this,
                width,
                height,
                length);
    }

    public void destroy() {
        nativeDestroyBox(mNativeRef);
    }

    private native long nativeCreateBox(ClassLoader appClassLoader, BoxJni boxJni,
                                        long width, long height, long length);
    private native void nativeDestroyBox(long nodeReference);
}
