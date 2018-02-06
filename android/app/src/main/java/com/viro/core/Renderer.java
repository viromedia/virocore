/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
/*
 * This class is a convenience wrapper around:
 *
 * Cpp JNI wrapper      : VRORenderer_JNI.cpp
 * Cpp Object           : VROSceneRendererGVR.cpp
 */
package com.viro.core;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetManager;
import android.view.Surface;

import com.google.ar.core.Session;
import com.viro.core.internal.PlatformUtil;

/**
 * @hide
 */
public class Renderer {

    protected long mNativeRef;
    private CameraListener mCameraListener;

    /* ----------     Scene view only methods    ---------- */

    public Renderer(ClassLoader appClassLoader, Context context, ViroViewScene view, AssetManager assets,
                    PlatformUtil platformUtil, RendererConfiguration config) {
        mNativeRef = nativeCreateRendererSceneView(appClassLoader, context, view, assets, platformUtil,
                config.isShadowsEnabled(), config.isHDREnabled(), config.isPBREnabled(), config.isBloomEnabled());
    }

    /* ----------     GVR only methods    ---------- */

    public Renderer(ClassLoader appClassLoader, Context context,
                    AssetManager assets, PlatformUtil platformUtil, long nativeGvrContext,
                    RendererConfiguration config) {
        mNativeRef = nativeCreateRendererGVR(appClassLoader, context, assets, platformUtil, nativeGvrContext,
                config.isShadowsEnabled(), config.isHDREnabled(), config.isPBREnabled(), config.isBloomEnabled());
    }

    public void drawFrame() {
        nativeDrawFrame(mNativeRef);
    }
    public void setVRModeEnabled(boolean enabled) { nativeSetVRModeEnabled(mNativeRef, enabled); }

    /* ----------     OVR only methods    ---------- */
    public Renderer(ClassLoader appClassLoader, Context context,
                    ViroViewOVR view, Activity activity, AssetManager assets, PlatformUtil platformUtil,
                    RendererConfiguration config) {
        mNativeRef = nativeCreateRendererOVR(appClassLoader, context, view, activity, assets, platformUtil,
                config.isShadowsEnabled(), config.isHDREnabled(), config.isPBREnabled(), config.isBloomEnabled());
    }

    public void onSurfaceDestroyed(Surface surface) { nativeOnSurfaceDestroyed(mNativeRef); }

    public void recenterTracking() { nativeRecenterTracking(mNativeRef); }

    /* ----------     ARCore only methods    ---------- */

    public Renderer(ClassLoader appClassLoader, Context context,
                    ViroViewARCore view, Session session, AssetManager assets, PlatformUtil platformUtil,
                    RendererConfiguration config) {
        mNativeRef = nativeCreateRendererARCore(appClassLoader, context, view, session, assets, platformUtil,
                config.isShadowsEnabled(), config.isHDREnabled(), config.isPBREnabled(), config.isBloomEnabled());
    }

    public int getCameraTextureId() {
        return nativeGetCameraTextureId(mNativeRef);
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

    public Vector getHitRay(float x, float y) {
        float[] ray = nativeGetHitRay(mNativeRef, x, y);
        if (ray != null) {
            return new Vector(ray);
        }
        else {
            return null;
        }
    }

    /* ----------     Common lifecycle methods    ---------- */

    public void destroy() {
        if (mNativeRef != 0) {
            nativeDestroyRenderer(mNativeRef);
        }
        mNativeRef = 0;
    }

    public void initalizeGl() { nativeInitializeGl(mNativeRef); }

    public void onStart() {
        if (mNativeRef != 0) {
            nativeOnStart(mNativeRef);
        }
    }

    public void onPause() {
        if (mNativeRef != 0) {
            nativeOnPause(mNativeRef);
        }
    }
    public void onResume() {
        if (mNativeRef != 0) {
            nativeOnResume(mNativeRef);
        }
    }

    public void onStop() {
        if (mNativeRef != 0) {
            nativeOnStop(mNativeRef);
        }
    }

    public void onSurfaceCreated(Surface surface) { nativeOnSurfaceCreated(surface, mNativeRef); }
    public void onSurfaceChanged(Surface surface, int width, int height) { nativeOnSurfaceChanged(surface, width, height, mNativeRef); }

    /* ----------     Common other methods   ---------- */

    public void onKeyEvent(int keyCode, int action) {
        nativeOnKeyEvent(mNativeRef, keyCode, action);
    }
    public void onTouchEvent(int onTouchAction, float touchPosX, float touchPosY) {
        nativeOnTouchEvent(mNativeRef, onTouchAction, touchPosX, touchPosY);
    }

    public void onPinchEvent(int pinchState, float scaleFactor, float viewportX, float viewportY) {
        nativeOnPinchEvent(mNativeRef, pinchState, scaleFactor, viewportX, viewportY);
    }

    public void onRotateEvent(int rotateState, float rotateRadians, float viewportX, float viewportY) {
        nativeOnRotateEvent(mNativeRef, rotateState, rotateRadians, viewportX, viewportY);
    }

    public void setSceneController(long nativeSceneControllerRef) {
        nativeSetSceneController(mNativeRef, nativeSceneControllerRef);
    }
    public void setSceneController(long nativeSceneControllerRef, float duration) {
        nativeSetSceneControllerWithAnimation(mNativeRef, nativeSceneControllerRef, duration);
    };
    public void setPointOfView(Node node) {
        if (node != null) {
            nativeSetPointOfView(mNativeRef, node.mNativeRef);
        }
        else {
            nativeSetPointOfView(mNativeRef, 0);
        }
    }

    public void setClearColor(int color) {
        nativeSetClearColor(mNativeRef, color);
    }
    public String getHeadset() {
        return nativeGetHeadset(mNativeRef);
    }
    public String getController() {
        return nativeGetController(mNativeRef);
    }
    public void setSuspended(boolean suspend) {
       nativeSetSuspended(mNativeRef, suspend);
    }
    public void setDebugHUDEnabled(boolean enabled) { nativeSetDebugHUDEnabled(mNativeRef, enabled); }

    public void addFrameListener(NativeFrameListener frameListener) {
        nativeAddFrameListener(mNativeRef, frameListener.mNativeRef);
    }

    public void removeFrameListener(NativeFrameListener frameListener) {
        nativeRemoveFrameListener(mNativeRef, frameListener.mNativeRef);
    }

    public Vector getLastCameraPositionRealtime() {
        return new Vector(nativeGetCameraPositionRealtime(mNativeRef));
    }

    public Vector getLastCameraRotationRealtime() {
        return new Vector(nativeGetCameraRotationRealtime(mNativeRef));
    }

    public Vector getLastCameraForwardRealtime() {
        return new Vector(nativeGetCameraForwardRealtime(mNativeRef));
    }

    public float getFieldOfView() { return nativeGetFieldOfView(mNativeRef); }

    public void setCameraListener(CameraListener listener) {
        nativeSetCameraListener(mNativeRef, listener != null);
        mCameraListener = listener;
    }

    private void onCameraTransformationUpdate(float[] pos, float[] rotEuler, float[] forward) {
        if (mCameraListener == null) {
            return;
        }

        Vector vPos = new Vector(pos);
        Vector vRotEuler = new Vector(rotEuler);
        Vector vForward = new Vector(forward);
        mCameraListener.onTransformUpdate(vPos, vRotEuler, vForward);
    }

    void setShadowsEnabled(boolean enabled) { nativeSetShadowsEnabled(mNativeRef, enabled); }
    void setHDREnabled(boolean enabled) { nativeSetHDREnabled(mNativeRef, enabled); }
    void setPBREnabled(boolean enabled) { nativeSetPBREnabled(mNativeRef, enabled); }
    void setBloomEnabled(boolean enabled) { nativeSetBloomEnabled(mNativeRef, enabled); }

    /* ----------     Native methods    ---------- */

    private native long nativeCreateRendererGVR(ClassLoader appClassLoader, Context context,
                                                AssetManager assets, PlatformUtil platformUtil, long nativeGvrContext,
                                                boolean enableShadows, boolean enableHDR, boolean enablePBR, boolean enableBloom);
    private native long nativeCreateRendererOVR(ClassLoader appClassLoader, Context context,
                                                ViroViewOVR view, Activity activity, AssetManager assets, PlatformUtil platformUtil,
                                                boolean enableShadows, boolean enableHDR, boolean enablePBR, boolean enableBloom);
    private native long nativeCreateRendererARCore(ClassLoader appClassLoader, Context context,
                                                   ViroViewARCore view, Session session, AssetManager assets, PlatformUtil platformUtil,
                                                   boolean enableShadows, boolean enableHDR, boolean enablePBR, boolean enableBloom);
    private native long nativeCreateRendererSceneView(ClassLoader appClassLoader, Context context,
                                                      ViroViewScene view, AssetManager assets, PlatformUtil platformUtil,
                                                      boolean enableShadows, boolean enableHDR, boolean enablePBR, boolean enableBloom);
    private native void nativeDestroyRenderer(long nativeRenderer);
    private native void nativeInitializeGl(long nativeRenderer);
    private native void nativeSetVRModeEnabled(long nativeRenderer, boolean enabled);
    private native long nativeDrawFrame(long nativeRenderer);
    private native void nativeOnStart(long nativeRenderer);
    private native void nativeOnKeyEvent(long nativeRenderer, int keyCode, int action);
    private native void nativeOnTouchEvent(long nativeRenderer, int onTouchAction, float touchPosX, float touchPosY);
    private native void nativeOnPinchEvent(long nativeRenderer, int pinchState, float scaleFactor, float viewportX, float viewportY);
    private native void nativeOnRotateEvent(long nativeRenderer, int rotateState, float rotateRadians, float viewportX, float viewportY);
    private native void nativeOnPause(long nativeRenderer);
    private native void nativeOnResume(long nativeRenderer);
    private native void nativeOnStop(long nativeRenderer);
    private native void nativeOnSurfaceCreated(Surface surface, long nativeRenderer);
    private native void nativeOnSurfaceChanged(Surface surface, int width, int height, long nativeRenderer);
    private native void nativeOnSurfaceDestroyed(long nativeRenderer);
    private native void nativeSetSceneController(long nativeRenderer, long nativeScene);
    private native void nativeSetSceneControllerWithAnimation(long nativeRenderer, long nativeScene, float duration);
    private native void nativeSetPointOfView(long nativeRenderer, long nodeRef);
    private native String nativeGetHeadset(long nativeRenderer);
    private native String nativeGetController(long nativeRenderer);
    private native void nativeSetDebugHUDEnabled(long nativeRenderer, boolean enabled);
    private native void nativeSetSuspended(long nativeRenderer, boolean suspendRenderer);
    private native void nativeRecenterTracking(long nativeRenderer);
    private native int nativeGetCameraTextureId(long nativeRenderer);
    private native void nativePerformARHitTestWithRay(long nativeRenderer, float[] ray, ARHitTestListener callback);
    private native void nativePerformARHitTestWithPosition(long nativeRenderer, float[] position, ARHitTestListener callback);
    private native void nativePerformARHitTestWithPoint(long nativeRenderer, float x, float y, ARHitTestListener callback);
    private native void nativeSetClearColor(long sceneRef, int color);
    private native void nativeSetShadowsEnabled(long nativeRef, boolean enabled);
    private native void nativeSetHDREnabled(long nativeRef, boolean enabled);
    private native void nativeSetPBREnabled(long nativeRef, boolean enabled);
    private native void nativeSetBloomEnabled(long nativeRef, boolean enabled);
    private native void nativeAddFrameListener(long nativeRenderer, long portalTraversalListener);
    private native void nativeRemoveFrameListener(long nativeRenderer, long portalTraversalListener);
    private native float[] nativeGetCameraPositionRealtime(long nativeRenderer);
    private native float[] nativeGetCameraRotationRealtime(long nativeRenderer);
    private native float[] nativeGetCameraForwardRealtime(long nativeRenderer);
    private native void nativeSetCameraListener(long nativeRenderer, boolean enabled);
    private native float[] nativeGetHitRay(long nativeRenderer, float x, float y);
    private native float nativeGetFieldOfView(long nativeRef);
}
