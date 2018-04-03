/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
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
        } else {
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