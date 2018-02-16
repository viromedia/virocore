/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * @hide
 */
public class SoundData {
    protected final long mNativeRef;

    public SoundData(String path) {
        mNativeRef = nativeCreateSoundData(path);
    }

    public void destroy() {
        nativeDestroySoundData(mNativeRef);
    }

    private native long nativeCreateSoundData(String path);
    private native void nativeDestroySoundData(long nativeRef);
}
