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

/**
 * @hide
 */
//#IFDEF 'viro_react'
public class AnimationGroup extends ExecutableAnimation {

    private LazyMaterial mLazyMaterial;

    public AnimationGroup(String positionX, String positionY, String positionZ,
                          String scaleX, String scaleY, String scaleZ,
                          String rotateX, String rotateY, String rotateZ,
                          String opacity, String color, LazyMaterial lazyMaterial,
                          float durationSeconds,
                          float delaySeconds, String functionType) {

        long lazyMaterialRef = 0;
        if (lazyMaterial != null) {
            mLazyMaterial = lazyMaterial;
            lazyMaterialRef = lazyMaterial.getNativeRef();
        }

        mNativeRef = nativeCreateAnimationGroup(positionX, positionY, positionZ,
                scaleX, scaleY, scaleZ,
                rotateX, rotateY, rotateZ,
                opacity, color, lazyMaterialRef,
                durationSeconds, delaySeconds, functionType);
        mDurationSeconds = nativeGetDuration(mNativeRef);
    }

    private AnimationGroup(long nativeRef, LazyMaterial lazyMaterial) {
        mNativeRef = nativeRef;
        if (lazyMaterial != null) {
            mLazyMaterial = lazyMaterial.copy();
        }
    }

    public long getNativeRef() {
        return mNativeRef;
    }

    @Override
    public ExecutableAnimation copy() {
        return new AnimationGroup(nativeCopyAnimation(mNativeRef), mLazyMaterial);
    }

    @Override
    public void dispose() {
        if (mLazyMaterial != null) {
            mLazyMaterial.destroy();
        }

        if (mNativeRef != 0) {
            nativeDestroyAnimationGroup(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreateAnimationGroup(String positionX, String positionY, String positionZ,
                                                   String scaleX, String scaleY, String scaleZ,
                                                   String rotateX, String rotateY, String rotateZ,
                                                   String opacity, String color, long lazyMaterialRef,
                                                   float durationSeconds, float delaySeconds, String functionType);
    private native long nativeCopyAnimation(long nativeRef);
    private native void nativeDestroyAnimationGroup(long nativeRef);
}
//#ENDIF
