/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class ImageTrackerOutput {
    private final long mNativeRef;

    public ImageTrackerOutput(long nativeRef) {
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
