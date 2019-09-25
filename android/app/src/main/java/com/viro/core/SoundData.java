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

package com.viro.core;

/**
 * @hide
 */
//#IFDEF 'viro_react'
public class SoundData {
    protected final long mNativeRef;
    protected final long mNativeDelegateRef;
    private SoundDataInitializeCallback mInitCallback;

    public SoundData(String path, SoundDataInitializeCallback callback) {
        mNativeRef = nativeCreateSoundData(path);
        mInitCallback = callback;

        if (mInitCallback != null) {
            mNativeDelegateRef = nativeSetSoundDataDelegate(mNativeRef);
        }
        else {
            // Don't reformat this block back to } else {, unless you know how to fix the regex in
            // preprocessor.gradle lines 23-27. I tried.
            mNativeDelegateRef = -1L;
        }
    }

    public void destroy() {
        if (mInitCallback != null) {
            nativeDestroySoundDataDelegate(mNativeDelegateRef);
            mInitCallback = null;
        }
        nativeDestroySoundData(mNativeRef);
    }

    private void dataError(String error) {
        if (mInitCallback != null) {
            mInitCallback.onDataError(error);
        }
    }

    private void dataIsReady() {
        if (mInitCallback != null) {
            mInitCallback.onDataIsReady();
        }
    }

    private native long nativeCreateSoundData(String path);
    private native void nativeDestroySoundData(long nativeRef);
    private native long nativeSetSoundDataDelegate(long nativeRef);
    private native long nativeDestroySoundDataDelegate(long nativeRef);

    public interface SoundDataInitializeCallback {
        void onDataError(String error);
        void onDataIsReady();
    }
}
//#ENDIF