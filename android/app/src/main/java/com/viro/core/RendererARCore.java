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
        nativeOnARCoreInstalled(mNativeRef, context); }

    public void setARDisplayGeometry(int rotation, int width, int height) {
        nativeSetARDisplayGeometry(mNativeRef, rotation, width, height);
    }

    public void setAnchorDetectionTypes(EnumSet<ViroViewARCore.AnchorDetectionType> types) {
        if (types.size() == 0) {
            nativeSetPlaneFindingMode(mNativeRef, false);
        }
        for (ViroViewARCore.AnchorDetectionType type : types) {
            if (type == ViroViewARCore.AnchorDetectionType.NONE) {
                nativeSetPlaneFindingMode(mNativeRef, false);
            } else if (type == ViroViewARCore.AnchorDetectionType.PLANES_HORIZONTAL) {
                nativeSetPlaneFindingMode(mNativeRef, true);
            }
        }
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

    private native long nativeCreateRendererARCore(ClassLoader appClassLoader, Context context,
                                                   AssetManager assets, PlatformUtil platformUtil,
                                                   boolean enableShadows, boolean enableHDR, boolean enablePBR, boolean enableBloom);
    private native void nativeSetARDisplayGeometry(long nativeRef, int rotation, int width, int height);
    private native void nativeSetPlaneFindingMode(long nativeRef, boolean enabled);
    private native void nativeOnARCoreInstalled(long nativeRef, Context context);
    private native int nativeGetCameraTextureId(long nativeRenderer);
    private native void nativePerformARHitTestWithRay(long nativeRenderer, float[] ray, ARHitTestListener callback);
    private native void nativePerformARHitTestWithPosition(long nativeRenderer, float[] position, ARHitTestListener callback);
    private native void nativePerformARHitTestWithPoint(long nativeRenderer, float x, float y, ARHitTestListener callback);

}
