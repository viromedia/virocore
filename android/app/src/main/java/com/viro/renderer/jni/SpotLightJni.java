/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class SpotLightJni extends BaseLight {

    public SpotLightJni(long color, float intensity, float attenuationStartDistance,
                        float attenuationEndDistance, float[] position, float[] direction,
                        float innerAngle, float outerAngle) {
        mNativeRef = nativeCreateSpotLight(color, intensity, attenuationStartDistance, attenuationEndDistance,
                position[0], position[1], position[2], direction[0], direction[1], direction[2],
                innerAngle, outerAngle);
    }

    public void destroy() {
        nativeDestroySpotLight(mNativeRef);
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

    public void setAttenuationStartDistance(float attenuationStartDistance) {

        nativeSetAttenuationStartDistance(mNativeRef, attenuationStartDistance);
    }

    public void setAttenuationEndDistance(float attenuationEndDistance) {

        nativeSetAttenuationEndDistance(mNativeRef, attenuationEndDistance);
    }

    public void setPosition(float[] position) {

        nativeSetPosition(mNativeRef, position[0], position[1], position[2]);
    }

    public void setDirection(float[] direction) {

        nativeSetDirection(mNativeRef, direction[0], direction[1], direction[2]);
    }

    public void setInnerAngle(float innerAngle) {

        nativeSetInnerAngle(mNativeRef, innerAngle);
    }

    public void setOuterAngle(float outerAngle) {

        nativeSetOuterAngle(mNativeRef, outerAngle);
    }

    public void setInfluenceBitMask(int bitMask) {
        nativeSetInfluenceBitMask(mNativeRef, bitMask);
    }

    public void setShadowOpacity(float opacity) {
        nativeSetShadowOpacity(mNativeRef, opacity);
    }

    public void setCastsShadow(boolean castsShadow) {
        nativeSetCastsShadow(mNativeRef, castsShadow);
    }

    public void setShadowMapSize(int shadowMapSize) {
        nativeSetShadowMapSize(mNativeRef, shadowMapSize);
    }

    public void setShadowBias(float shadowBias) {
        nativeSetShadowBias(mNativeRef, shadowBias);
    }

    public void setShadowNearZ(float shadowNearZ) {
        nativeSetShadowNearZ(mNativeRef, shadowNearZ);
    }

    public void setShadowFarZ(float shadowFarZ) {
        nativeSetShadowFarZ(mNativeRef, shadowFarZ);
    }

    private native long nativeCreateSpotLight(long color, float intensity,
                                              float attenuationStartDistance,
                                              float attenuationEndDistance,
                                              float positionX, float positionY, float positionZ,
                                              float directionX, float directionY, float directionZ,
                                              float innerAngle, float outerAngle);
    private native void nativeDestroySpotLight(long lightRef);
    private native void nativeAddToNode(long lightRef, long nodeRef);
    private native void nativeRemoveFromNode(long lightRef, long nodeRef);
    private native void nativeSetColor(long lightRef, long color);
    private native void nativeSetIntensity(long lightRef, float intensity);
    private native void nativeSetAttenuationStartDistance(long lightRef, float attenuationStartDistance);
    private native void nativeSetAttenuationEndDistance(long lightRef, float attenuationEndDistance);
    private native void nativeSetPosition(long lightRef, float positionX, float positionY, float positionZ);
    private native void nativeSetDirection(long lightRef, float directionX, float directionY, float directionZ);
    private native void nativeSetInnerAngle(long lightRef, float innerAngle);
    private native void nativeSetOuterAngle(long lightRef, float outerAngle);

    private native void nativeSetInfluenceBitMask(long lightRef, int bitMask);
    private native void nativeSetShadowMapSize(long lightRef, int shadowMapSize);
    private native void nativeSetShadowBias(long lightRef, float shadowBias);
    private native void nativeSetShadowNearZ(long lightRef, float shadowNearZ);
    private native void nativeSetShadowFarZ(long lightRef, float shadowFarZ);
    private native void nativeSetCastsShadow(long lightRef, boolean castsShadow);
    private native void nativeSetShadowOpacity(long lightRef, float opacity);
}

