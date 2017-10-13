/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;

/**
 * @hide
 */
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
