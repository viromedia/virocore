/*
 * Copyright © 2016 Viro Media. All rights reserved.
 */
/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.Box.java
 * Java JNI Wrapper     : com.viro.renderer.BoxJni.java
 * Cpp JNI wrapper      : Box_JNI.cpp
 * Cpp Object           : VROBox.cpp
 */
package com.viro.renderer.jni;

/**
 * Box is a simple shape representing a six-sided geometry, defined by width, height, and length.
 */
public class Box extends Geometry {

    /**
     * Construct a new Box with the given width, height, and length.
     *
     * @param width  The width of the box (X dimension).
     * @param height The height of the box (Y dimension).
     * @param length The length of the box (Z dimension).
     */
    public Box(float width, float height, float length) {
        mNativeRef = nativeCreateBox(width, height, length);
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
     * Release native resources associated with this Box.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyBox(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Set the width of this Box.
     *
     * @param width The size of the Box in the X dimension.
     */
    public void setWidth(float width) {
        nativeSetWidth(mNativeRef, width);
    }

    /**
     * Set the height of this Box.
     *
     * @param height The size of the Box in the Y dimension.
     */
    public void setHeight(float height) {
        nativeSetHeight(mNativeRef, height);
    }

    /**
     * Set the length of this Box.
     *
     * @param length The size of the Box in the Z dimension.
     */
    public void setLength(float length) {
        nativeSetLength(mNativeRef, length);
    }

    private native long nativeCreateBox(float width, float height, float length);
    private native void nativeDestroyBox(long nodeReference);
    private native void nativeSetWidth(long boxReference, float width);
    private native void nativeSetHeight(long boxReference, float height);
    private native void nativeSetLength(long boxReference, float length);

}
