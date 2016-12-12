/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.content.Context;
import android.content.res.AssetManager;

/**
 * This class is a convenience wrapper around:
 *
 * Cpp JNI wrapper      : VRORenderer_JNI.cpp
 * Cpp Object           : VROSceneRendererCardboard.cpp
 */
public class RendererJni {
    protected long mNativeRef;

    public RendererJni(ClassLoader appClassLoader, VrView view, Context context,
                       AssetManager assets, long nativeGvrContext) {
        mNativeRef = nativeCreateRenderer(appClassLoader, view, context, assets, nativeGvrContext);
    }

    public void destroy() {
        nativeDestroyRenderer(mNativeRef);
    }

    public void initalizeGl() {
        nativeInitializeGl(mNativeRef);
    }

    public void drawFrame() {
        nativeDrawFrame(mNativeRef);
    }

    public void onTriggerEvent() {
        nativeOnTriggerEvent(mNativeRef);
    }

    public void setScene(long nativeSceneRef) {
        nativeSetScene(mNativeRef, nativeSceneRef);
    }

    public void onPause() {
        nativeOnPause(mNativeRef);
    }

    public void onResume() {
        nativeOnResume(mNativeRef);
    }

    public void enableReticle(boolean enable) {
        nativeEnableReticle(mNativeRef, enable);
    }

    public void setCameraPosition(float[] position) {
        nativeSetCameraPosition(mNativeRef, position[0], position[1], position[2]);
    }

    public void setCameraRotationType(String rotationType) {
        nativeSetCameraRotationType(mNativeRef, rotationType);
    }

    public void setOrbitCameraFocalPoint(float[] position) {
        nativeSetOrbitCameraFocalPoint(mNativeRef, position[0], position[1], position[2]);
    }

    private native long nativeCreateRenderer(ClassLoader appClassLoader, VrView view, Context context,
                                             AssetManager assets, long nativeGvrContext);
    private native void nativeDestroyRenderer(long nativeRenderer);
    private native void nativeInitializeGl(long nativeRenderer);
    private native long nativeDrawFrame(long nativeRenderer);
    private native void nativeOnTriggerEvent(long nativeRenderer);
    private native void nativeOnPause(long nativeRenderer);
    private native void nativeOnResume(long nativeRenderer);
    private native void nativeSetScene(long nativeRenderer, long nativeScene);
    private native void nativeEnableReticle(long nativeRenderer, boolean enable);
    private native void nativeSetCameraPosition(long nativeRenderer, float x, float y, float z);
    private native void nativeSetCameraRotationType(long nativeRenderer, String rotationType);
    private native void nativeSetOrbitCameraFocalPoint(long nativeRenderer, float x, float y, float z);
}
