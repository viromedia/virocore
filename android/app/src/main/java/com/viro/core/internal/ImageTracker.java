//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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