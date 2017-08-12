/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class AmbientLightJni extends BaseLight {

    public AmbientLightJni(long color, float intensity) {
        mNativeRef = nativeCreateAmbientLight(color, intensity);
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

    public void setIntensity(float intensity) {
        nativeSetIntensity(mNativeRef, intensity);
    }

    private native long nativeCreateAmbientLight(long color, float intensity);

    private native void nativeDestroyAmbientLight(long lightRef);

    private native void nativeAddToNode(long lightRef, long nodeRef);

    private native void nativeRemoveFromNode(long lightRef, long nodeRef);

    private native void nativeSetColor(long lightRef, long color);

    private native void nativeSetIntensity(long lightRef, float intensity);
}
