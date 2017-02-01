/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.Controller.java
 * Java JNI Wrapper     : com.viro.renderer.ControllerJni.java
 * Cpp JNI wrapper      : Controller_JNI.cpp
 * Cpp Object           : VROInputPresenter
 */
public class ControllerJni {
    private RenderContextJni mRenderContext;
    public ControllerJni(RenderContextJni renderContext) {
        mRenderContext = renderContext;
    }

    public void setEventDelegate(EventDelegateJni delegate) {
        nativeSetEventDelegate(mRenderContext.mNativeRef, delegate.mNativeRef);
    }

    public void enableReticle(boolean enable) {
        nativeEnableReticle(mRenderContext.mNativeRef, enable);
    }

    private native void nativeSetEventDelegate(long contextRef, long delegateRef);
    private native void nativeEnableReticle(long contextRef, boolean enabled);
}
