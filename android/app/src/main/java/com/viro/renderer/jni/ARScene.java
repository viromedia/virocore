/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import java.lang.ref.WeakReference;

public class ARScene extends Scene {

    public interface ARSceneDelegate {
        void onTrackingInitialized();
        void onAmbientLightUpdate(float lightIntensity, float colorTemperature);
        void onAnchorFound(ARAnchor anchor);
        void onAnchorUpdated(ARAnchor anchor);
        void onAnchorRemoved(ARAnchor anchor);
    }

    private WeakReference<ARSceneDelegate> mARSceneDelegate = null;
    private long mNativeARDelegateRef;
    private boolean mDeclarative;

    /**
     * Construct a new ARScene.
     */
    public ARScene() {
        mNativeARDelegateRef = nativeCreateARSceneDelegate(mNativeRef);
    }

    /**
     * Constructor for the declarative session.
     *
     * @hide
     * @param declarative
     */
    public ARScene(boolean declarative) {
        super(true); // Invoke the dummy constructor
        long nativeRef = nativeCreateARSceneControllerDeclarative();
        setSceneRef(nativeRef);
        mNativeARDelegateRef = nativeCreateARSceneDelegate(mNativeRef);
    }

    /**
     * Invoked only by the default (no-arg) constructor.
     *
     * @hide
     */
    @Override
    protected long createNativeScene() {
        return nativeCreateARSceneController();
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this ARScene.
     */
    @Override
    public void dispose() {
        super.dispose();
        if (mNativeARDelegateRef != 0) {
            nativeDestroyARSceneDelegate(mNativeARDelegateRef);
            mNativeARDelegateRef = 0;
        }
    }

    public void displayPointCloud(boolean displayPointCloud) {
        nativeDisplayPointCloud(mNativeRef, displayPointCloud);
    }

    public void resetPointCloudSurface() {
        nativeResetPointCloudSurface(mNativeRef);
    }

    public void setPointCloudSurface(Surface surface) {
        nativeSetPointCloudSurface(mNativeRef, surface.mNativeRef);
    }

    public void setPointCloudSurfaceScale(Vector scale) {
        nativeSetPointCloudSurfaceScale(mNativeRef, scale.x, scale.y, scale.z);
    }

    public void setPointCloudMaxPoints(int maxPoints) {
        nativeSetPointCloudMaxPoints(mNativeRef, maxPoints);
    }

    /**
     * @hide
     */
    public void addARDeclarativeNode(ARDeclarativeNode node) {
        nativeAddARNode(mNativeRef, node.mNativeRef);
    }

    /**
     * @hide
     */
    public void updateARDeclarativeNode(ARDeclarativeNode node) {
        nativeUpdateARNode(mNativeRef, node.mNativeRef);
    }
    /**
     * @hide
     */
    public void removeARDeclarativeNode(ARDeclarativeNode node) {
        nativeRemoveARNode(mNativeRef, node.mNativeRef);
    }

    private native long nativeCreateARSceneController();
    private native long nativeCreateARSceneControllerDeclarative();
    private native long nativeCreateARSceneDelegate(long sceneControllerRef);
    private native void nativeDestroyARSceneDelegate(long delegateRef);
    private native void nativeAddARNode(long sceneControllerRef, long arPlaneRef);
    private native void nativeUpdateARNode(long sceneControllerRef, long arPlaneRef);
    private native void nativeRemoveARNode(long sceneControllerRef, long arPlaneRef);
    private native void nativeDisplayPointCloud(long sceneControllerRef, boolean displayPointCloud);
    private native void nativeResetPointCloudSurface(long sceneControllerRef);
    private native void nativeSetPointCloudSurface(long sceneControllerRef, long surfaceRef);
    private native void nativeSetPointCloudSurfaceScale(long sceneControllerRef, float scaleX, float scaleY, float scaleZ);
    private native void nativeSetPointCloudMaxPoints(long sceneControllerRef, int maxPoints);

    public void registerARDelegate(ARSceneDelegate delegate) {
        mARSceneDelegate = new WeakReference<ARSceneDelegate>(delegate);
    }

    // Called by JNI

    /**
     * @hide
     */
    void onTrackingInitialized() {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onTrackingInitialized();
        }
    }
    /**
     * @hide
     */
    void onAmbientLightUpdate(float lightIntensity, float colorTemperature) {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onAmbientLightUpdate(lightIntensity, colorTemperature);
        }
    }
    /**
     * @hide
     */
    void onAnchorFound(ARAnchor anchor) {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onAnchorFound(anchor);
        }
    }
    /**
     * @hide
     */
    void onAnchorUpdated(ARAnchor anchor) {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onAnchorUpdated(anchor);
        }
    }
    /**
     * @hide
     */
    /* Called by Native */
    void onAnchorRemoved(ARAnchor anchor) {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onAnchorRemoved(anchor);
        }
    }
}
