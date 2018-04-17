/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;

import com.viro.core.ARImageTarget;

/**
 * @hide
 */
//#IFDEF 'viro_react'
// This class is meant to create a singular instance of VROARImageTracker and manually
// run frames through it w/ findTarget(Bitmap). Should only be used for debug.
public class ImageTracker {

    private ARImageTarget mImageTarget;
    private long mNativeRef;

    public ImageTracker(ARImageTarget imageTarget) {
        mImageTarget = imageTarget;
        mNativeRef = nativeCreateImageTracker(imageTarget.getNativeRef());
    }

    public ImageTrackerOutput findTarget(Bitmap image) {
        return new ImageTrackerOutput(nativeFindTarget(mNativeRef, image));
    }

    private native long nativeCreateImageTracker(long imageTargetRef);
    private native long nativeFindTarget(long nativeRef, Bitmap bitmap);
}
//#ENDIF