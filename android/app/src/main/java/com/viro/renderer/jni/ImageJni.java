/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


import android.graphics.Bitmap;

public class ImageJni {
    protected long mNativeRef;

    public ImageJni(String resource) {
        mNativeRef = nativeCreateImage(resource);
    }

    public ImageJni(Bitmap bitmap) {
        mNativeRef = nativeCreateImageFromBitmap(bitmap);
    }

    public long getWidth() {
        return nativeGetWidth(mNativeRef);
    }

    public long getHeight() {
        return nativeGetHeight(mNativeRef);
    }

    public void destroy() {
        nativeDestroyImage(mNativeRef);
    }

    private native long nativeCreateImage(String resource);
    private native long nativeCreateImageFromBitmap(Bitmap bitmap);
    private native long nativeGetWidth(long nativeRef);
    private native long nativeGetHeight(long nativeRef);
    private native void nativeDestroyImage(long nativeRef);
}
