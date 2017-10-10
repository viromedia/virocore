/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.Box.java
 * Java JNI Wrapper     : com.viro.renderer.BoxJni.java
 * Cpp JNI wrapper      : Box_JNI.cpp
 * Cpp Object           : VROBox.cpp
 */
public class Box extends BaseGeometry {
    public Box(float width, float height, float length) {
        mNativeRef = nativeCreateBox(width, height, length);
    }

    public void destroy() {
        nativeDestroyBox(mNativeRef);
    }

    @Override
    public void attachToNode(Node node) {
        nativeAttachToNode(mNativeRef, node.mNativeRef);
    }

    public void setWidth(float width) { nativeSetWidth(mNativeRef, width); }
    public void setHeight(float height) { nativeSetHeight(mNativeRef, height); }
    public void setLength(float length) { nativeSetLength(mNativeRef, length); }

    private native long nativeCreateBox(float width, float height, float length);
    private native void nativeDestroyBox(long nodeReference);
    private native void nativeAttachToNode(long boxReference, long nodeReference);
    private native void nativeSetWidth(long boxReference, float width);
    private native void nativeSetHeight(long boxReference, float height);
    private native void nativeSetLength(long boxReference, float length);
}
