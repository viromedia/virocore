/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import com.viro.renderer.ARAnchor;

import java.lang.ref.WeakReference;

public class ARScene extends Scene {
    public long mNativeARDelegateRef;

    public ARScene() {
        mNativeARDelegateRef = nativeCreateARSceneDelegate(mNativeRef);
    }

    @Override
    protected long createNativeScene() {
        return nativeCreateARSceneController();
    }

    @Override
    public void dispose() {
        nativeDestroySceneControllerDelegate(mNativeDelegateRef); // comes from SceneControllerJni
        nativeDestroyARSceneDelegate(mNativeARDelegateRef);
        nativeDestroyARSceneController(mNativeRef);
    }

    public void displayPointCloud(boolean displayPointCloud) {
        nativeDisplayPointCloud(mNativeRef, displayPointCloud);
    }

    public void addARPlane(ARPlane arPlane) {
        nativeAddARPlane(mNativeRef, arPlane.mNativeRef);
    }

    public void updateARPlane(ARPlane arPlane) {
        nativeUpdateARPlane(mNativeRef, arPlane.mNativeRef);
    }

    public void removeARPlane(ARPlane arPlane) {
        nativeRemoveARPlane(mNativeRef, arPlane.mNativeRef);
    }

    private native long nativeCreateARSceneController();

    private native long nativeCreateARSceneDelegate(long sceneControllerRef);

    private native void nativeDestroyARSceneController(long sceneControllerRef);

    private native void nativeDestroyARSceneDelegate(long delegateRef);

    private native void nativeDisplayPointCloud(long sceneControllerRef, boolean displayPointCloud);

    private native void nativeAddARPlane(long sceneControllerRef, long arPlaneRef);

    private native void nativeUpdateARPlane(long sceneControllerRef, long arPlaneRef);

    private native void nativeRemoveARPlane(long sceneControllerRef, long arPlaneRef);

    // -- ARSceneDelegate --

    private WeakReference<ARSceneDelegate> mARSceneDelegate = null;

    public interface ARSceneDelegate {
        void onTrackingInitialized();
        void onAmbientLightUpdate(float lightIntensity, float colorTemperature);
        void onAnchorFound(ARAnchor anchor);
        void onAnchorUpdated(ARAnchor anchor);
        void onAnchorRemoved(ARAnchor anchor);
    }

    public void registerARDelegate(ARSceneDelegate delegate) {
        mARSceneDelegate = new WeakReference<ARSceneDelegate>(delegate);
    }

    /* Called by Native */
    public void onTrackingInitialized() {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onTrackingInitialized();
        }
    }

    /* Called by Native */
    public void onAmbientLightUpdate(float lightIntensity, float colorTemperature) {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onAmbientLightUpdate(lightIntensity, colorTemperature);
        }
    }

    /* Called by Native */
    public void onAnchorFound(ARAnchor anchor) {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onAnchorFound(anchor);
        }
    }

    /* Called by Native */
    public void onAnchorUpdated(ARAnchor anchor) {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onAnchorUpdated(anchor);
        }
    }

    /* Called by Native */
    public void onAnchorRemoved(ARAnchor anchor) {
        ARSceneDelegate delegate;
        if (mARSceneDelegate != null && (delegate = mARSceneDelegate.get()) != null) {
            delegate.onAnchorRemoved(anchor);
        }
    }
}
