/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.jni.SurfaceJni.java
 * Cpp JNI wrapper      : Surface_JNI.cpp
 * Cpp Object           : VROSurface.cpp
 */
package com.viro.core;

/**
 * Surface represents a one-sided plane defined by a width and height.
 */
public class Surface extends Geometry {

    private float mWidth, mHeight;

    /**
     * Construct a new Surface with the given width and height.
     *
     * @param width  The extent of the Surface along its horizontal axis (X).
     * @param height The extent of the Surface along its vertical axis (Y).
     */
    public Surface(float width, float height) {
        this(width, height, 0, 0, 1, 1);
    }

    /**
     * @hide
     */
    public Surface(float width, float height, float u0, float v0, float u1, float v1) {
        mWidth = width;
        mHeight = height;
        mNativeRef = nativeCreateSurface(width, height, u0, v0, u1, v1);
    }

    /**
     * @hide
     */
    public Surface(float width, float height, float u0, float v0, float u1, float v1,
                   Surface oldSurface) {
        mNativeRef = nativeCreateSurfaceFromSurface(width, height, u0, v0, u1, v1,
                oldSurface.mNativeRef);
    }

    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this Surface.
     */
    public void dispose(){
        if (mNativeRef != 0) {
            nativeDestroySurface(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Set the width of this Surface.
     *
     * @param width The extent of the Surface along its horizontal axis.
     */
    public void setWidth(float width) {
        mWidth = width;
        nativeSetWidth(mNativeRef, width);
    }

    /**
     * Get the width of this Surface.
     *
     * @return The extent of this Surface along its horizontal axis.
     */
    public float getWidth() {
        return mWidth;
    }

    /**
     * Set the height of this Surface.
     *
     * @param height The extent of the Surface along its vertical axis.
     */
    public void setHeight(float height) {
        mHeight = height;
        nativeSetHeight(mNativeRef, height);
    }

    /**
     * Get the height of this surface.
     *
     * @return The extent of this Surface along its vertical axis.
     */
    public float getHeight() {
        return mHeight;
    }

    private native long nativeCreateSurface(float width, float height, float u0, float v0, float u1, float v1);
    private native long nativeCreateSurfaceFromSurface(float width, float height,
                                                       float u0, float v0, float u1, float v1,
                                                       long oldSurfaceRef);
    private native void nativeDestroySurface(long surfaceRef);
    private native void nativeSetWidth(long surfaceRef, float width);
    private native void nativeSetHeight(long surfaceRef, float height);
    private native void nativeSetVideoTexture(long surfaceRef, long textureRef);
    private native void nativeSetImageTexture(long surfaceRef, long textureRef);
    private native void nativeSetMaterial(long surfaceRef, long materialRef);
    private native void nativeClearMaterial(long surfaceRef);

    /**
     * @hide
     * @param texture
     */
    public void setVideoTexture(VideoTexture texture){
        nativeSetVideoTexture(mNativeRef, texture.mNativeRef);
    }
    /**
     * @hide
     * @param texture
     */
    public void setImageTexture(Texture texture) {
        nativeSetImageTexture(mNativeRef, texture.mNativeRef);
    }
    /**
     * @hide
     * @param material
     */
    public void setMaterial(Material material) {
        nativeSetMaterial(mNativeRef, material.mNativeRef);
    }
    /**
     * @hide
     */
    public void clearMaterial() {
        nativeClearMaterial(mNativeRef);
    }

}
