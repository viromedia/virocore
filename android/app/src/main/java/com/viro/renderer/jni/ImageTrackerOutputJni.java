/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class ImageTrackerOutputJni {
    private final long mNativeRef;

    public ImageTrackerOutputJni(long nativeRef) {
        mNativeRef = nativeRef;
    }

    public boolean found() {
        return nativeOutputFound(mNativeRef);
    }

    public float[] corners() {
        return nativeOutputCorners(mNativeRef);
    }

    private native boolean nativeOutputFound(long nativeRef);
    private native float[] nativeOutputCorners(long nativeRef);
}
