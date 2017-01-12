/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class DirectionalLightJni extends BaseLight {

    public DirectionalLightJni(long color, float[] direction) {
        mNativeRef = nativeCreateDirectionalLight(color, direction[0], direction[1], direction[2]);
    }

    public void destroy() {
        nativeDestroyDirectionalLight(mNativeRef);
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

    public void setDirection(float[] direction) {

        nativeSetDirection(mNativeRef, direction[0], direction[1], direction[2]);
    }

    private native long nativeCreateDirectionalLight(long color, float directionX, float directionY,
                                                     float directionZ);
    private native void nativeDestroyDirectionalLight(long lightRef);
    private native void nativeAddToNode(long lightRef, long nodeRef);
    private native void nativeRemoveFromNode(long lightRef, long nodeRef);
    private native void nativeSetColor(long lightRef, long color);
    private native void nativeSetDirection(long lightRef, float directionX, float directionY, float directionZ);
}
