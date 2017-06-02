/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;

public class ImageTrackerJni {

    private Context mContext;
    private Bitmap mTargetImage;
    private long mNativeRef;

    public ImageTrackerJni(Context context, Bitmap targetImage) {
        mContext = context;
        mTargetImage = targetImage;
        mNativeRef = nativeCreateImageTracker(targetImage);
    }

    public void findTarget(Bitmap image) {
        ImageTrackerOutputJni output = new ImageTrackerOutputJni(nativeFindTarget(mNativeRef, image));

        if (output.found()) {
            for (float f : output.corners()) {
                Log.i("ImageTrackerJni", "corners are: " + f);
            }
        }
    }

    private native long nativeCreateImageTracker(Bitmap targetImage);
    private native long nativeFindTarget(long nativeRef, Bitmap bitmap);
}
