/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class OmniLight extends BaseLight {

    public OmniLight(long color, float intensity, float attenuationStartDistance,
                     float attenuationEndDistance, float[] position) {
        mNativeRef = nativeCreateOmniLight(color, intensity, attenuationStartDistance,
                attenuationEndDistance, position[0], position[1], position[2]);
    }

    public void destroy() {
        nativeDestroyOmniLight(mNativeRef);
    }

    @Override
    public void addToNode(Node nodeJni) {
        nativeAddToNode(mNativeRef, nodeJni.mNativeRef);
    }

    @Override
    public void removeFromNode(Node nodeJni) {
        nativeRemoveFromNode(mNativeRef, nodeJni.mNativeRef);
    }

    public void setColor(long color) {

        nativeSetColor(mNativeRef, color);
    }

    public void setIntensity(float intensity) {
        nativeSetIntensity(mNativeRef, intensity);
    }

    public void setAttenuationStartDistance(float attenuationStartDistance) {

        nativeSetAttenuationStartDistance(mNativeRef, attenuationStartDistance);
    }

    public void setAttenuationEndDistance(float attenuationEndDistance) {

        nativeSetAttenuationEndDistance(mNativeRef, attenuationEndDistance);
    }

    public void setPosition(float[] position) {

        nativeSetPosition(mNativeRef, position[0], position[1], position[2]);
    }

    public void setInfluenceBitMask(int bitMask) {
        nativeSetInfluenceBitMask(mNativeRef, bitMask);
    }

    private native long nativeCreateOmniLight(long color, float intensity,
                                              float attenuationStartDistance,
                                              float attenuationEndDistance,
                                              float positionX, float positionY, float positionZ);
    private native void nativeDestroyOmniLight(long lightRef);
    private native void nativeAddToNode(long lightRef, long nodeRef);
    private native void nativeRemoveFromNode(long lightRef, long nodeRef);
    private native void nativeSetColor(long lightRef, long color);
    private native void nativeSetIntensity(long lightRef, float intensity);
    private native void nativeSetAttenuationStartDistance(long lightRef, float attenuationStartDistance);
    private native void nativeSetAttenuationEndDistance(long lightRef, float attenuationEndDistance);
    private native void nativeSetPosition(long lightRef, float positionX, float positionY, float positionZ);
    private native void nativeSetInfluenceBitMask(long lightRef, int bitMask);

}
