/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.renderer.jni;

import android.util.Log;

import java.lang.ref.WeakReference;

/**
 * @hide
 */
public class ARDeclarativeNode extends ARNode {

    public interface Delegate {
        void onAnchorFound(ARAnchor anchor);
        void onAnchorUpdated(ARAnchor anchor);
        void onAnchorRemoved();
    }

    private WeakReference<Delegate> mARNodeDelegate = null;
    long mNativeARNodeDelegateRef;

    public ARDeclarativeNode() {

    }

    @Override
    protected void setNativeRef(long nativeRef) {
        super.setNativeRef(nativeRef);
        mNativeARNodeDelegateRef = nativeCreateARNodeDelegate(nativeRef);
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
     * Release native resources associated with this ARNode.
     */
    @Override
    public void dispose() {
        super.dispose();
        if (mNativeARNodeDelegateRef != 0) {
            nativeDestroyARNodeDelegate(mNativeARNodeDelegateRef);
            mNativeARNodeDelegateRef = 0;
        }
    }

    public void setDelegate(Delegate delegate) {
        mARNodeDelegate = new WeakReference<Delegate>(delegate);
    }

    public void setAnchorId(String anchorId) {
        nativeSetAnchorId(mNativeRef, anchorId);
    }

    private native void nativeSetAnchorId(long nativeRef, String anchorId);
    private native long nativeCreateARNodeDelegate(long nativeRef);
    private native void nativeDestroyARNodeDelegate(long delegateRef);

    // Invoked by Native

    /**
     * @hide
     */
    void onAnchorFound(ARAnchor anchor) {
        Delegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorFound(anchor);
        }
    }
    /**
     * @hide
     */
    void onAnchorUpdated(ARAnchor anchor) {
        Delegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorUpdated(anchor);
        }
    }
    /**
     * @hide
     */
    void onAnchorRemoved() {
        Delegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorRemoved();
        }
    }

}
