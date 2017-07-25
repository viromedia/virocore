/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class ExecutableAnimationJni {
    public enum ExecutionType {
        SERIAL, PARALLEL
    }

    protected long mNativeRef;

    /**
     * Constructor used by subclasses. The subclass must initialize the
     * mNativeRef.
     */
    protected ExecutableAnimationJni() {}

    /**
     * Create an ExecutableAnimationJni wrapping the given animation
     * contained in the node.
     */
    public ExecutableAnimationJni(NodeJni node, String key) {
        mNativeRef = nativeWrapNodeAnimation(node.mNativeRef, key);
    }

    public void execute(NodeJni node, AnimationDelegate delegate) {
        mDelegate = delegate;
        execute(node);
    }

    public ExecutableAnimationJni copy() {
        throw new UnsupportedOperationException();
    }

    public void execute(NodeJni node) {
        nativeExecuteAnimation(mNativeRef, node.mNativeRef);
    }
    public void pause() {
        nativePauseAnimation(mNativeRef);
    }
    public void resume() {
        nativeResumeAnimation(mNativeRef);
    }
    public void terminate() {
        nativeTerminateAnimation(mNativeRef);
    }

    public void destroy() { nativeDestroyAnimation(mNativeRef); }

    /**
     * AnimationDelegate logic
     */

    protected AnimationDelegate mDelegate;

    public interface AnimationDelegate {
        void onFinish(ExecutableAnimationJni animation);
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
    private native void nativeTerminateAnimation(long nativeRef);
    private native void nativeDestroyAnimation(long nativeRef);
}
