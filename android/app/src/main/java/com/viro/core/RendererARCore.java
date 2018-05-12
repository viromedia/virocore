/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.content.Context;
import android.content.res.AssetManager;

import com.viro.core.internal.PlatformUtil;

import java.util.EnumSet;

/**
 * @hide
 */
public class RendererARCore extends Renderer {

    public RendererARCore(ClassLoader appClassLoader, Context context, AssetManager assets, PlatformUtil platformUtil,
                          RendererConfiguration config) {
        mNativeRef = nativeCreateRendererARCore(appClassLoader, context, assets, platformUtil,
                config.isShadowsEnabled(), config.isHDREnabled(), config.isPBREnabled(), config.isBloomEnabled());
    }

    public int getCameraTextureId() {
        return nativeGetCameraTextureId(mNativeRef);
    }

    public void onARCoreInstalled(Context context) {
        // This is done in two steps because we create the ARCore session in libviro_arcore.so
        // then inject it into the scene renderer from libviro_native.so. The two libraries are
        // not linked so we need to perform the injection using reinterpret_cast. We do not link
        // the libraries so that we can run libviro_native on older devices (libviro_arcore requires
        // Android 24)
        long sessionRef = nativeCreateARCoreSession(context);
        nativeSetARCoreSession(mNativeRef, sessionRef);
    }

    public void setARDisplayGeometry(int rotation, int width, int height) {
        nativeSetARDisplayGeometry(mNativeRef, rotation, width, height);
    }

    public void setAnchorDetectionTypes(EnumSet<ViroViewARCore.AnchorDetectionType> types) {
        String[] detectionTypes = new String[types.size()];
        int i = 0;
        for (ViroViewARCore.AnchorDetectionType type : types) {
            detectionTypes[i] = type.getStringValue();
            i++;
        }

        nativeSetAnchorDetectionTypes(mNativeRef, detectionTypes);
    }

    public void performARHitTestWithRay(float[] ray, ARHitTestListener callback) {
        nativePerformARHitTestWithRay(mNativeRef, ray, callback);
    }

    public void performARHitTestWithPosition(float[] position, ARHitTestListener callback) {
        nativePerformARHitTestWithPosition(mNativeRef, position, callback);
    }

    public void performARHitTestWithPoint(float x, float y, ARHitTestListener callback) {
        nativePerformARHitTestWithPoint(mNativeRef, x, y, callback);
    }

    public void enableTracking(boolean shouldTrack) {
        nativeEnableTracking(mNativeRef, shouldTrack);
    }

    private native long nativeCreateRendererARCore(ClassLoader appClassLoader, Context context,
                                                   AssetManager assets, PlatformUtil platformUtil,
                                                   boolean enableShadows, boolean enableHDR, boolean enablePBR, boolean enableBloom);
    private native void nativeSetARDisplayGeometry(long nativeRef, int rotation, int width, int height);
    private native void nativeSetAnchorDetectionTypes(long nativeRef, String[] detectionTypes);
    private native long nativeCreateARCoreSession(Context context);
    private native void nativeSetARCoreSession(long nativeRef, long sessionRef);
    private native int nativeGetCameraTextureId(long nativeRenderer);
    private native void nativePerformARHitTestWithRay(long nativeRenderer, float[] ray, ARHitTestListener callback);
    private native void nativePerformARHitTestWithPosition(long nativeRenderer, float[] position, ARHitTestListener callback);
    private native void nativePerformARHitTestWithPoint(long nativeRenderer, float x, float y, ARHitTestListener callback);
    private native void nativeEnableTracking(long nativeRenderer, boolean shouldTrack);

}
