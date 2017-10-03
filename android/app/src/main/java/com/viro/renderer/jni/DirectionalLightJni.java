/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class DirectionalLightJni extends BaseLight {

    public DirectionalLightJni(long color, float intensity, float[] direction) {
        mNativeRef = nativeCreateDirectionalLight(color, intensity, direction[0], direction[1], direction[2]);
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

    public void setIntensity(float intensity) {
        nativeSetIntensity(mNativeRef, intensity);
    }

    public void setDirection(float[] direction) {

        nativeSetDirection(mNativeRef, direction[0], direction[1], direction[2]);
    }

    public void setCastsShadow(boolean castsShadow) {
        nativeSetCastsShadow(mNativeRef, castsShadow);
    }

    public void setInfluenceBitMask(int bitMask) {
        nativeSetInfluenceBitMask(mNativeRef, bitMask);
    }

    public void setShadowOrthographicSize(float orthographicSize) {
        nativeSetShadowOrthographicSize(mNativeRef, orthographicSize);
    }

    public void setShadowOrthographicPosition(float [] position) {
        nativeSetShadowOrthographicPosition(mNativeRef, position[0], position[1], position[2]);
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

    public void setShadowOpacity(float opacity) {
        nativeSetShadowOpacity(mNativeRef, opacity);
    }


    private native long nativeCreateDirectionalLight(long color, float intensity,
                                                     float directionX, float directionY, float directionZ);
    private native void nativeDestroyDirectionalLight(long lightRef);

    private native void nativeAddToNode(long lightRef, long nodeRef);
    private native void nativeRemoveFromNode(long lightRef, long nodeRef);
    private native void nativeSetColor(long lightRef, long color);
    private native void nativeSetIntensity(long lightRef, float intensity);
    private native void nativeSetDirection(long lightRef, float directionX, float directionY, float directionZ);


    private native void nativeSetCastsShadow(long lightRef, boolean castsShadow);
    private native void nativeSetInfluenceBitMask(long lightRef, int bitMask);
    private native void nativeSetShadowOrthographicSize(long lightRef, float size);
    private native void nativeSetShadowOrthographicPosition(long lightRef, float posX, float posY, float posZ);
    private native void nativeSetShadowMapSize(long lightRef, int shadowMapSize);
    private native void nativeSetShadowBias(long lightRef, float shadowBias);
    private native void nativeSetShadowNearZ(long lightRef, float shadowNearZ);
    private native void nativeSetShadowFarZ(long lightRef, float shadowFarZ);
    private native void nativeSetShadowOpacity(long lightRef, float opacity);
}
