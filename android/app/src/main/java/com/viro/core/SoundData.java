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

    public SoundData(String path, boolean local) {
        mNativeRef = nativeCreateSoundData(path, local);
    }

    public void destroy() {
        nativeDestroySoundData(mNativeRef);
    }

    private native long nativeCreateSoundData(String path, boolean local);
    private native void nativeDestroySoundData(long nativeRef);
}
