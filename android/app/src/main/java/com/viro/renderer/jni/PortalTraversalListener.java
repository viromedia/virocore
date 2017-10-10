package com.viro.renderer.jni;

/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

public class PortalTraversalListener extends FrameListener{

    public PortalTraversalListener(SceneController sceneControllerJni) {
        mNativeRef = nativeCreatePortalTraversalListener(sceneControllerJni.mNativeRef);
    }

    public void destroy() {
        nativeDestroyPortalTraversalListener(mNativeRef);
    }

    private native long nativeCreatePortalTraversalListener(long sceneControllerRef);
    private native void nativeDestroyPortalTraversalListener(long nativeRef);
}
