/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import android.util.Log;

import com.viro.core.Node;

/**
 * @hide
 */
public class ExecutableAnimation {
    public enum ExecutionType {
        SERIAL, PARALLEL
    }

    protected long mNativeRef;
    protected float mDurationSeconds;
    protected float mTimeOffsetSeconds;
    protected float mSpeed;
    protected AnimationDelegate mDelegate;

    /**
     * Constructor used by subclasses. The subclass must initialize the
     * mNativeRef.
     */
    protected ExecutableAnimation() {}

    /**
     * Create an ExecutableAnimationJni wrapping the given animation
     * contained in the node.
     */
    public ExecutableAnimation(Node node, String key) {
        mNativeRef = nativeWrapNodeAnimation(node.getNativeRef(), key);
        mDurationSeconds = nativeGetDuration(mNativeRef);
    }

    public ExecutableAnimation copy() {
        throw new UnsupportedOperationException();
    }

    public void setDuration(float durationSeconds) {
        mDurationSeconds = durationSeconds;
        nativeSetDuration(mNativeRef, durationSeconds);
    }

    public void setTimeOffset(float timeOffset) {
        mTimeOffsetSeconds = timeOffset;
        nativeSetTimeOffset(mNativeRef, timeOffset);
    }

    public void setSpeed(float speed) {
        mSpeed = speed;
        nativeSetSpeed(mNativeRef, speed);
    }

    public float getDuration() {
        return mDurationSeconds;
    }

    public void execute(Node node, AnimationDelegate delegate) {
        mDelegate = delegate;
        execute(node);
    }

    public void execute(Node node) {
        if (node.getNativeRef() == 0) {
            return;
        }
        nativeExecuteAnimation(mNativeRef, node.getNativeRef());
    }

    public void pause() {
        nativePauseAnimation(mNativeRef);
    }
    public void resume() {
        nativeResumeAnimation(mNativeRef);
    }
    public void terminate(boolean jumpToEnd) {
        nativeTerminateAnimation(mNativeRef, jumpToEnd);
    }
    public void destroy() { nativeDestroyAnimation(mNativeRef); }

    /**
     * AnimationDelegate
     */
    public interface AnimationDelegate {
        void onFinish(ExecutableAnimation animation);
    }

    public void animationDidFinish() {
        if (mDelegate != null) {
            mDelegate.onFinish(this);
        }
    }

    private native long nativeWrapNodeAnimation(long nodeRef, String key);
    private native void nativeExecuteAnimation(long nativeRef, long nodeRef);
    private native void nativePauseAnimation(long nativeRef);
    private native void nativeResumeAnimation(long nativeRef);
    private native void nativeTerminateAnimation(long nativeRef, boolean allowInterruptable);
    private native void nativeSetSpeed(long nativeRef, float speed);
    private native void nativeSetDuration(long nativeRef, float durationSeconds);
    private native void nativeSetTimeOffset(long nativeRef, float timeOffset);
    private native void nativeDestroyAnimation(long nativeRef);

    // This should only be invoked on construction to get the initial duration for mDurationSeconds
    protected native float nativeGetDuration(long nativeRef);
}
