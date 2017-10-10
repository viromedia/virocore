/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


import android.graphics.Bitmap;

public class Image {
    protected long mNativeRef;

    public Image(String resource, TextureFormat format) {
        mNativeRef = nativeCreateImage(resource, format.getID());
    }

    public Image(Bitmap bitmap, TextureFormat format) {
        mNativeRef = nativeCreateImageFromBitmap(bitmap, format.getID());
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

    private native long nativeCreateImage(String resource, String format);
    private native long nativeCreateImageFromBitmap(Bitmap bitmap, String format);
    private native long nativeGetWidth(long nativeRef);
    private native long nativeGetHeight(long nativeRef);
    private native void nativeDestroyImage(long nativeRef);
}
