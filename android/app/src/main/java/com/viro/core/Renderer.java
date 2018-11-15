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

import com.viro.core.internal.PlatformUtil;

import java.lang.ref.WeakReference;

/**
 * @hide
 */
public class Renderer {

    protected long mNativeRef;
    private CameraListener mCameraListener;
    private FrameListener mFrameListener;
    protected NativeFrameListenerImpl mNativeFrameListener;

    protected static class NativeFrameListenerImpl extends NativeFrameListener {
        WeakReference<Renderer> mWeakRenderer;

        public NativeFrameListenerImpl(Renderer renderer) {
            mWeakRenderer = new WeakReference<Renderer>(renderer);
            mNativeRef = renderer.nativeCreateFrameListener(renderer.mNativeRef);
        }

        @Override
        public void destroy() {
            if (mNativeRef != 0) {
                Renderer renderer = mWeakRenderer.get();
                if (renderer != null) {
                    renderer.nativeDestroyFrameListener(mNativeRef);
                }
            }
            mNativeRef = 0;
        }
    }

    protected Renderer() {

    }

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

    /* ----------     Common lifecycle methods    ---------- */

    public void destroy() {
        if (mNativeFrameListener != null) {
            mNativeFrameListener.destroy();
        }
        mNativeFrameListener = null;

        if (mNativeRef != 0) {
            nativeDestroyRenderer(mNativeRef);
        }
        mNativeRef = 0;
    }

    public void initializeGL(boolean framebufferSRGB) {
        nativeInitializeGL(mNativeRef, framebufferSRGB, ViroView.NATIVE_TESTING_MODE);
    }

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

    public void performHitTestWithPoint(int x, int y, boolean useBoundsOnly, HitTestListener callback) {
        nativePerformHitTestWithPoint(mNativeRef, x, y, useBoundsOnly, callback);
    }

    public void performARHitTestWithRay(float[] origin, float[] ray, boolean useBoundsOnly, HitTestListener callback) {
        nativePerformHitTestWithRay(mNativeRef, origin, ray, useBoundsOnly, callback);
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

    public Vector projectPoint(float x, float y, float z) {
        float[] pt = nativeProjectPoint(mNativeRef, x, y, z);
        if (pt != null) {
            return new Vector(pt);
        }
        else {
            return null;
        }
    }

    public Vector unprojectPoint(float x, float y, float z) {
        float[] pt = nativeUnprojectPoint(mNativeRef, x, y, z);
        if (pt != null) {
            return new Vector(pt);
        }
        else {
            return null;
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

    public void setFrameListener(FrameListener listener) {
        if (mNativeFrameListener == null) {
            mNativeFrameListener = new NativeFrameListenerImpl(this);
        }

        if (listener == null) {
            if (mFrameListener != null) {
                removeFrameListener(mNativeFrameListener);
            }
        } else {
            if (mFrameListener == null) {
                addFrameListener(mNativeFrameListener);
            }
        }
        mFrameListener = listener;
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

    public void onCameraTransformationUpdate(float[] pos, float[] rotEuler, float[] forward) {
        if (mCameraListener == null) {
            return;
        }
        Vector vPos = new Vector(pos);
        Vector vRotEuler = new Vector(rotEuler);
        Vector vForward = new Vector(forward);
        mCameraListener.onTransformUpdate(vPos, vRotEuler, vForward);
    }

    public void onFrameDidRender() {
        if (mFrameListener != null) {
            mFrameListener.onDrawFrame();
        }
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
    private native long nativeCreateRendererSceneView(ClassLoader appClassLoader, Context context,
                                                      ViroViewScene view, AssetManager assets, PlatformUtil platformUtil,
                                                      boolean enableShadows, boolean enableHDR, boolean enablePBR, boolean enableBloom);
    private native void nativeDestroyRenderer(long nativeRenderer);
    private native long nativeCreateFrameListener(long nativeRenderer);
    private native void nativeDestroyFrameListener(long frameListenerRef);
    private native void nativeInitializeGL(long nativeRenderer, boolean sRGBFramebuffer, boolean testingMode);
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
    private native void nativeSetClearColor(long sceneRef, int color);
    private native void nativeSetShadowsEnabled(long nativeRef, boolean enabled);
    private native void nativeSetHDREnabled(long nativeRef, boolean enabled);
    private native void nativeSetPBREnabled(long nativeRef, boolean enabled);
    private native void nativeSetBloomEnabled(long nativeRef, boolean enabled);
    private native void nativeAddFrameListener(long nativeRenderer, long frameListener);
    private native void nativeRemoveFrameListener(long nativeRenderer, long frameListener);
    private native float[] nativeGetCameraPositionRealtime(long nativeRenderer);
    private native float[] nativeGetCameraRotationRealtime(long nativeRenderer);
    private native float[] nativeGetCameraForwardRealtime(long nativeRenderer);
    private native void nativeSetCameraListener(long nativeRenderer, boolean enabled);
    private native float[] nativeProjectPoint(long nativeRenderer, float x, float y, float z);
    private native float[] nativeUnprojectPoint(long nativeRenderer, float x, float y, float z);
    private native float nativeGetFieldOfView(long nativeRef);
    private native void nativePerformHitTestWithPoint(long nativeRef, int x, int y, boolean boundsOnly, HitTestListener callback);
    private native void nativePerformHitTestWithRay(long nativeRef, float origin[], float ray[], boolean boundsOnly, HitTestListener callback);

}
