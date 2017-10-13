/*
 * Copyright © 2016 Viro Media. All rights reserved.
 */
/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.jni.SurfaceJni.java
 * Cpp JNI wrapper      : Surface_JNI.cpp
 * Cpp Object           : VROSurface.cpp
 */
package com.viro.renderer.jni;

/**
 * Surface represents a one-sided plane defined by a width and height.
 */
public class Surface extends Geometry {

    /**
     * Construct a new Surface with the given width and height.
     *
     * @param width  The extent of the Surface along its horizontal axis (X).
     * @param height The extent of the Surface along its vertical axis (Y).
     */
    public Surface(float width, float height) {
        this(width, height, 0, 1, 0, 1);
    }

    /**
     * @hide
     */
    public Surface(float width, float height, float u0, float v0, float u1, float v1) {
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

    public void setVideoTexture(VideoTexture texture){
        nativeSetVideoTexture(mNativeRef, texture.mNativeRef);
    }

    public void setImageTexture(Texture texture) {
        nativeSetImageTexture(mNativeRef, texture.mNativeRef);
    }

    public void setMaterial(Material material) {
        nativeSetMaterial(mNativeRef, material.mNativeRef);
    }

    /**
     * Set the width of this Surface.
     *
     * @param width The extent of the Surface along its horizontal axis.
     */
    public void setWidth(float width) {
        nativeSetWidth(mNativeRef, width);
    }

    /**
     * Set the height of this Surface.
     *
     * @param height The extent of the Surface along its vertical axis.
     */
    public void setHeight(float height) {
        nativeSetHeight(mNativeRef, height);
    }

    public void clearMaterial() {
        nativeClearMaterial(mNativeRef);
    }

    /*
     * Native Functions called into JNI
     */
    private native long nativeCreateSurface(float width, float height,
                                            float u0, float v0, float u1, float v1);
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

}
