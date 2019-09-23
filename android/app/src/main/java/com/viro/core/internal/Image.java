//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.core.internal;


import android.graphics.Bitmap;

import com.viro.core.Texture;

/**
 * @hide
 */
//#IFDEF 'viro_react'
public class Image {
    public long mNativeRef;

    public Image(String resource, Texture.Format format) {
        mNativeRef = nativeCreateImage(resource, format.getStringValue());
    }

    public Image(Bitmap bitmap, Texture.Format format) {
        mNativeRef = nativeCreateImageFromBitmap(bitmap, format.getStringValue());
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            destroy();
        }
        finally {
            super.finalize();
        }
    }

    public long getWidth() {
        return nativeGetWidth(mNativeRef);
    }

    public long getHeight() {
        return nativeGetHeight(mNativeRef);
    }

    public void destroy() {
        if (mNativeRef != 0) {
            nativeDestroyImage(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreateImage(String resource, String format);
    private native long nativeCreateImageFromBitmap(Bitmap bitmap, String format);
    private native int nativeGetWidth(long nativeRef);
    private native int nativeGetHeight(long nativeRef);
    private native void nativeDestroyImage(long nativeRef);
}
//#ENDIF