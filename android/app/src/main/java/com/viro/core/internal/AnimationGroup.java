/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
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
    public void destroy() {
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
