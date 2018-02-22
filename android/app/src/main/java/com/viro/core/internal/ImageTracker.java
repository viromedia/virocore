/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;

/**
 * @hide
 */
//#IFDEF 'viro_react'
public class ImageTracker {

    private Context mContext;
    private Bitmap mTargetImage;
    private long mNativeRef;

    public ImageTracker(Context context, Bitmap targetImage) {
        mContext = context;
        mTargetImage = targetImage;
        mNativeRef = nativeCreateImageTracker(targetImage);
    }

    public void findTarget(Bitmap image) {
        ImageTrackerOutput output = new ImageTrackerOutput(nativeFindTarget(mNativeRef, image));

        if (output.found()) {
            for (float f : output.corners()) {
                Log.i("ImageTrackerJni", "corners are: " + f);
            }
        }
    }

    private native long nativeCreateImageTracker(Bitmap targetImage);
    private native long nativeFindTarget(long nativeRef, Bitmap bitmap);
}
//#ENDIF