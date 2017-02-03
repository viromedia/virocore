/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


public class SoundDataJni {
    protected final long mNativeRef;

    public SoundDataJni(String path, boolean local) {
        mNativeRef = nativeCreateSoundData(path, local);
    }

    public void destroy() {
        nativeDestroySoundData(mNativeRef);
    }

    private native long nativeCreateSoundData(String path, boolean local);
    private native void nativeDestroySoundData(long nativeRef);
}
