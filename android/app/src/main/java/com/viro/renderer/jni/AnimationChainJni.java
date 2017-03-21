/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class AnimationChainJni extends BaseAnimation {

    public AnimationChainJni(ExecutionType type) {
        mNativeRef = nativeCreateAnimationChain(type.name());
    }

    private AnimationChainJni(long nativeRef) {
        mNativeRef = nativeRef;
    }

    public void addAnimation(AnimationGroupJni animationGroup) {
        nativeAddAnimationGroup(mNativeRef, animationGroup.mNativeRef);
    }

    public void addAnimation(AnimationChainJni animationChain) {
        nativeAddAnimationChain(mNativeRef, animationChain.mNativeRef);
    }

    @Override
    public BaseAnimation copy() {
        return new AnimationChainJni(nativeCopyAnimation(mNativeRef));
    }

    @Override
    protected void execute(NodeJni node) {
        nativeExecuteAnimation(mNativeRef, node.mNativeRef);
    }

    @Override
    public void pause() {
        nativePauseAnimation(mNativeRef);
    }

    @Override
    public void resume() {
        nativeResumeAnimation(mNativeRef);
    }

    @Override
    public void terminate() {
        nativeTerminateAnimation(mNativeRef);
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
    private native void nativeExecuteAnimation(long nativeRef, long nodeRef);
    private native void nativePauseAnimation(long nativeRef);
    private native void nativeResumeAnimation(long nativeRef);
    private native void nativeTerminateAnimation(long nativeRef);
    private native void nativeDestroyAnimationChain(long nativeRef);

}
