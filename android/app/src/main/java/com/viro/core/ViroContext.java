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

/*
 * Java JNI wrapper for linking the following classes across the bridge:
 *
 * Java JNI Wrapper     : com.viro.renderer.RenderContextJni.java
 * Cpp JNI wrapper      : RenderContext_JNI.cpp
 * Cpp Object           : RenderContext
 */
package com.viro.core;

import com.viro.core.internal.CameraCallback;

/**
 * ViroContext provides context for the Viro application. It is tied to a specific EGL Context
 * and is required for rendering tasks.
 */
public class ViroContext {

    long mNativeRef;
    private Controller mController;

    /**
     * Construct a new ViroContext with the native handle to the Renderer.
     *
     * @param mRenderRef The native peer.
     * @hide
     */
    ViroContext(long mRenderRef){
        mNativeRef = nativeCreateViroContext(mRenderRef);
        mController = new Controller(this);
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
     * Release native resources associated with this ViroContext.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDeleteViroContext(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreateViroContext(long mNativeRenderer);
    private native void nativeDeleteViroContext(long mNativeContextRef);
    private native void nativeGetCameraOrientation(long mNativeContextRef, CameraCallback callback);

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void getCameraOrientation(CameraCallback callback) {
        nativeGetCameraOrientation(mNativeRef, callback);
    }
    //#ENDIF
    
    /**
     * @hide
     */
    public Controller getController() {
        return mController;
    }

}