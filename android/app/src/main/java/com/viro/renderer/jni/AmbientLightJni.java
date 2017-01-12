/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class AmbientLightJni extends BaseLight {

    public AmbientLightJni(long color) {
        mNativeRef = nativeCreateAmbientLight(color);
    }

    public void destroy() {
        nativeDestroyAmbientLight(mNativeRef);
    }
    @Override
    public void addToNode(NodeJni nodeJni) {

        nativeAddToNode(mNativeRef, nodeJni.mNativeRef);
    }

    @Override
    public void removeFromNode(NodeJni nodeJni) {

        nativeRemoveFromNode(mNativeRef, nodeJni.mNativeRef);
    }

    public void setColor(long color) {

        nativeSetColor(mNativeRef, color);
    }

    private native long nativeCreateAmbientLight(long color);

    private native void nativeDestroyAmbientLight(long lightRef);

    private native void nativeAddToNode(long lightRef, long nodeRef);

    private native void nativeRemoveFromNode(long lightRef, long nodeRef);

    private native void nativeSetColor(long lightRef, long color);
}
