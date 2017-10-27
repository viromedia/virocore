/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


import android.graphics.Bitmap;

/**
 * @hide
 */
public class Image {
    protected long mNativeRef;

    public Image(String resource, Texture.TextureFormat format) {
        mNativeRef = nativeCreateImage(resource, format.getStringValue());
    }

    public Image(Bitmap bitmap, Texture.TextureFormat format) {
        mNativeRef = nativeCreateImageFromBitmap(bitmap, format.getStringValue());
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
