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

import com.viro.core.ARAnchor;
import com.viro.core.ARNode;

import java.lang.ref.WeakReference;

/**
 * @hide
 */
//#IFDEF 'viro_react'
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
    protected void initWithNativeRef(long nativeRef) {
        super.initWithNativeRef(nativeRef);
        mNativeARNodeDelegateRef = nativeCreateARNodeDelegate(nativeRef);
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        }
        finally {
            super.finalize();
        }
    }

    /** Release native resources associated with this ARNode. */
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

    /*
     Invoked by Native
      */

    void onAnchorFound(ARAnchor anchor) {
        Delegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorFound(anchor);
        }
    }
    void onAnchorUpdated(ARAnchor anchor) {
        Delegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorUpdated(anchor);
        }
    }

    void onAnchorRemoved() {
        Delegate delegate;
        if (mARNodeDelegate != null && (delegate = mARNodeDelegate.get()) != null) {
            delegate.onAnchorRemoved();
        }
    }

}
//#ENDIF