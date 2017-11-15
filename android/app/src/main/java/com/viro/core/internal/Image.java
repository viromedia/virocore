/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;


import android.graphics.Bitmap;

import com.viro.core.Texture;

/**
 * @hide
 */
public class Image {
    public long mNativeRef;

    public Image(String resource, Texture.Format format) {
        mNativeRef = nativeCreateImage(resource, format.getStringValue());
    }

    public Image(Bitmap bitmap, Texture.Format format) {
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
