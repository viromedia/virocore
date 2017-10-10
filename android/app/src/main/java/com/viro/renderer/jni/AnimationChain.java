/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class AnimationChain extends ExecutableAnimation {

    public AnimationChain(ExecutionType type) {
        mNativeRef = nativeCreateAnimationChain(type.name());
    }

    private AnimationChain(long nativeRef) {
        mNativeRef = nativeRef;
    }

    public void addAnimation(AnimationGroup animationGroup) {
        nativeAddAnimationGroup(mNativeRef, animationGroup.mNativeRef);
    }

    public void addAnimation(AnimationChain animationChain) {
        nativeAddAnimationChain(mNativeRef, animationChain.mNativeRef);
    }

    @Override
    public ExecutableAnimation copy() {
        return new AnimationChain(nativeCopyAnimation(mNativeRef));
    }

    @Override
    public void destroy() {
        if (mNativeRef != 0) {
            nativeDestroyAnimationChain(mNativeRef);
        }
        mNativeRef = 0;
    }

    private native long nativeCreateAnimationChain(String executionType);
    private native long nativeCopyAnimation(long nativeRef);
    private native void nativeAddAnimationChain(long nativeRef, long chainRef);
    private native void nativeAddAnimationGroup(long nativeRef, long groupRef);
    private native void nativeDestroyAnimationChain(long nativeRef);

}
