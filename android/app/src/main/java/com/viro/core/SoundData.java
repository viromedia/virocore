/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
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
