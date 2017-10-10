package com.viro.renderer.jni;

/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

public class Portal extends Node {

    public Portal() {
        // call the empty NodeJni constructor.
        super(false);
        setNativeRef(nativeCreatePortal());
    }

    public void destroy() {
        nativeDestroyPortal(mNativeRef);
    }

    private native long nativeCreatePortal();

    private native void nativeDestroyPortal(long portalRef);
}
