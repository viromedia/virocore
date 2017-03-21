/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public abstract class BaseAnimation {
    public enum ExecutionType {
        SERIAL, PARALLEL
    }

    protected long mNativeRef;

    public void execute(NodeJni node, AnimationDelegate delegate) {
        mDelegate = delegate;
        execute(node);
    }

    public abstract BaseAnimation copy();
    protected abstract void execute(NodeJni nodeJni);
    public abstract void pause();
    public abstract void resume();
    public abstract void terminate();
    public abstract void destroy();

    /**
     * AnimationDelegate logic
     */

    protected AnimationDelegate mDelegate;

    public interface AnimationDelegate {
        void onFinish(BaseAnimation animation);
    }

    public void animationDidFinish() {
        if (mDelegate != null) {
            mDelegate.onFinish(this);
        }
    }
}
