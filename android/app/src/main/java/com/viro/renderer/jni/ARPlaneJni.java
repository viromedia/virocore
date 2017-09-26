/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class ARPlaneJni extends NodeJni {

    public ARPlaneJni(float minWidth, float minHeight) {
        super(false); // call the dummy parent constructor.
        setNativeRef(nativeCreateARPlane(minWidth, minHeight));
    }

    public void destroy() {
        nativeDestroyARPlane(mNativeRef);
    }

    public void setMinWidth(float minWidth) {
        nativeSetMinWidth(mNativeRef, minWidth);
    }

    public void setMinHeight(float minHeight) {
        nativeSetMinHeight(mNativeRef, minHeight);
    }

    private native long nativeCreateARPlane(float minWidth, float minHeight);

    private native void nativeDestroyARPlane(long nativeRef);

    private native void nativeSetMinWidth(long nativeRef, float minWidth);

    private native void nativeSetMinHeight(long nativeRef, float minHeight);
}
