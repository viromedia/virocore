/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class AnimationGroupJni extends ExecutableAnimationJni {

    private LazyMaterialJni mLazyMaterial;

    public AnimationGroupJni(String positionX, String positionY, String positionZ,
                             String scaleX, String scaleY, String scaleZ,
                             String rotateX, String rotateY, String rotateZ,
                             String opacity, String color, LazyMaterialJni lazyMaterial,
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
    }

    private AnimationGroupJni(long nativeRef, LazyMaterialJni lazyMaterial) {
        mNativeRef = nativeRef;
        if (lazyMaterial != null) {
            mLazyMaterial = lazyMaterial.copy();
        }
    }

    @Override
    public ExecutableAnimationJni copy() {
        return new AnimationGroupJni(nativeCopyAnimation(mNativeRef), mLazyMaterial);
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
