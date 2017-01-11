/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.view.Surface;

/**
 * This class is a convenience wrapper around:
 *
 * Cpp JNI wrapper      : VRORenderer_JNI.cpp
 * Cpp Object           : VROSceneRendererCardboard.cpp
 */
public class RendererJni {

    protected long mNativeRef;

    /* ----------     GVR only methods    ---------- */

    public RendererJni(ClassLoader appClassLoader, Context context,
                       AssetManager assets, PlatformUtil platformUtil, long nativeGvrContext) {
        mNativeRef = nativeCreateRendererGVR(appClassLoader, context, assets, platformUtil, nativeGvrContext);
    }

    public void drawFrame() {
        nativeDrawFrame(mNativeRef);
    }

    /* ----------     OVR only methods    ---------- */

    public RendererJni(ClassLoader appClassLoader, Context context,
                       ViroOvrView view, Activity activity, AssetManager assets, PlatformUtil platformUtil) {
        mNativeRef = nativeCreateRendererOVR(appClassLoader, context, view, activity, assets, platformUtil);
    }

    public void onSurfaceDestroyed(Surface surface) { nativeOnSurfaceDestroyed(mNativeRef); }

    /* ----------     Common lifecycle methods    ---------- */

    public void destroy() { nativeDestroyRenderer(mNativeRef); }
    public void initalizeGl() { nativeInitializeGl(mNativeRef); }

    public void onStart() { nativeOnStart(mNativeRef); }
    public void onPause() {
        nativeOnPause(mNativeRef);
    }
    public void onResume() {
        nativeOnResume(mNativeRef);
    }
    public void onStop() { nativeOnStop(mNativeRef); }
    public void onSurfaceCreated(Surface surface) { nativeOnSurfaceCreated(surface, mNativeRef); }
    public void onSurfaceChanged(Surface surface) { nativeOnSurfaceChanged(surface, mNativeRef); }

    /* ----------     Common other methods   ---------- */

    public void onTriggerEvent() {
        nativeOnTriggerEvent(mNativeRef);
    }
    public void setScene(long nativeSceneRef) {
        nativeSetScene(mNativeRef, nativeSceneRef);
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

    /* ----------     Native methods    ---------- */

    private native long nativeCreateRendererGVR(ClassLoader appClassLoader, Context context,
                                                AssetManager assets, PlatformUtil platformUtil, long nativeGvrContext);
    private native long nativeCreateRendererOVR(ClassLoader appClassLoader, Context context,
                                                ViroOvrView view, Activity activity, AssetManager assets, PlatformUtil platformUtil);
    private native void nativeDestroyRenderer(long nativeRenderer);
    private native void nativeInitializeGl(long nativeRenderer);
    private native long nativeDrawFrame(long nativeRenderer);
    private native void nativeOnTriggerEvent(long nativeRenderer);
    private native void nativeOnStart(long nativeRenderer);
    private native void nativeOnPause(long nativeRenderer);
    private native void nativeOnResume(long nativeRenderer);
    private native void nativeOnStop(long nativeRenderer);
    private native void nativeOnSurfaceCreated(Surface surface, long nativeRenderer);
    private native void nativeOnSurfaceChanged(Surface surface, long nativeRenderer);
    private native void nativeOnSurfaceDestroyed(long nativeRenderer);
    private native void nativeSetScene(long nativeRenderer, long nativeScene);
    private native void nativeEnableReticle(long nativeRenderer, boolean enable);
    private native void nativeSetCameraPosition(long nativeRenderer, float x, float y, float z);
    private native void nativeSetCameraRotationType(long nativeRenderer, String rotationType);
    private native void nativeSetOrbitCameraFocalPoint(long nativeRenderer, float x, float y, float z);
}
