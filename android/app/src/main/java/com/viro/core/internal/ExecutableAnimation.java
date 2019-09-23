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

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this animation.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyAnimation(mNativeRef);
            mNativeRef = 0;
        }
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
