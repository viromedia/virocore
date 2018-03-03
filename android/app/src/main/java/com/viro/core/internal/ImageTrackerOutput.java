/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

/**
 * @hide
 */
//#IFDEF 'viro_react'
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

    public float[] position() {
        return nativeOutputPosition(mNativeRef);
    }

    public float[] rotation() {
        return nativeOutputRotation(mNativeRef);
    }

    private native boolean nativeOutputFound(long nativeRef);
    private native float[] nativeOutputCorners(long nativeRef);
    private native float[] nativeOutputPosition(long nativeRef);
    private native float[] nativeOutputRotation(long nativeRef);
}
//#ENDIF