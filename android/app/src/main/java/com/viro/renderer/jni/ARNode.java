/*
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import com.viro.renderer.ARAnchor;

import java.lang.ref.WeakReference;

public abstract class ARNode extends Node {

    public long mNativeARNodeDelegateRef;

    public ARNode() {
        super(false); // call the empty NodeJni constructor.
    }

    public void destroy() {
        destroyNativeARNodeDelegate(mNativeARNodeDelegateRef);
    }

    protected void setNativeRef(long nativeRef) {
        super.setNativeRef(nativeRef);
        mNativeARNodeDelegateRef = createNativeARNodeDelegate(nativeRef);
    }

    // We want our child classes to create their own ARNodeDelegate objects so they can
    // handle creating the ARAnchor objects in the C++ JNI layer. But this class still
    // implements the callbacks so that we don't have to duplicate more code.
    abstract long createNativeARNodeDelegate(long nativeRef);
    abstract void destroyNativeARNodeDelegate(long delegateRef);

    // -- ARNodeDelegate --

    public interface ARNodeDelegate {
        void onAnchorFound(ARAnchor anchor);
        void onAnchorUpdated(ARAnchor anchor);
        void onAnchorRemoved();
    }

    private WeakReference<ARNodeDelegate> mARNodeDelegate = null;

    public void registerARNodeDelegate(ARNodeDelegate delegate) {
        mARNodeDelegate = new WeakReference<ARNodeDelegate>(delegate);
    }

    /* Called by Native */
    public void onAnchorFound(ARAnchor anchor) {
        ARNodeDelegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorFound(anchor);
        }
    }

    /* Called by Native */
    public void onAnchorUpdated(ARAnchor anchor) {
        ARNodeDelegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorUpdated(anchor);
        }
    }

    /* Called by Native */
    public void onAnchorRemoved() {
        ARNodeDelegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorRemoved();
        }
    }
}
