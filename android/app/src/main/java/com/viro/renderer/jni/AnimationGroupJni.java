/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class AnimationGroupJni extends BaseAnimation {

    public AnimationGroupJni(String positionX, String positionY, String positionZ,
                             String scaleX, String scaleY, String scaleZ,
                             String rotateX, String rotateY, String rotateZ,
                             String opacity, String color, float durationSeconds,
                             float delaySeconds, String functionType) {
        mNativeRef = nativeCreateAnimationGroup(positionX, positionY, positionZ,
                scaleX, scaleY, scaleZ,
                rotateX, rotateY, rotateZ,
                opacity, color, durationSeconds, delaySeconds, functionType);
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

    public void destroy() {
        nativeDestroyAnimationGroup(mNativeRef);
    }

    private native long nativeCreateAnimationGroup(String positionX, String positionY, String positionZ,
                                                   String scaleX, String scaleY, String scaleZ,
                                                   String rotateX, String rotateY, String rotateZ,
                                                   String opacity, String color, float durationSeconds,
                                                   float delaySeconds, String functionType);
    private native void nativeExecuteAnimation(long nativeRef, long nodeRef);
    private native void nativePauseAnimation(long nativeRef);
    private native void nativeResumeAnimation(long nativeRef);
    private native void nativeTerminateAnimation(long nativeRef);
    private native void nativeDestroyAnimationGroup(long nativeRef);
}
