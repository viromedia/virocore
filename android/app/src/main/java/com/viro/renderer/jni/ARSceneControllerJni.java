/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import java.lang.ref.WeakReference;

public class ARSceneControllerJni extends SceneControllerJni {
    public long mNativeARDelegateRef;

    public ARSceneControllerJni(NodeJni node) {
        super();
        setSceneRef(nativeCreateARSceneController(node.mNativeRef));
        mNativeARDelegateRef = nativeCreateARSceneDelegate(mNativeRef);
    }

    @Override
    public void destroy() {
        nativeDestroySceneControllerDelegate(mNativeDelegateRef); // comes from SceneControllerJni
        nativeDestroyARSceneDelegate(mNativeARDelegateRef);
        nativeDestroyARSceneController(mNativeRef);
    }

    public void addARPlane(ARPlaneJni arPlane) {
        nativeAddARPlane(mNativeRef, arPlane.mNativeRef);
    }

    public void updateARPlane(ARPlaneJni arPlane) {
        nativeUpdateARPlane(mNativeRef, arPlane.mNativeRef);
    }

    public void removeARPlane(ARPlaneJni arPlane) {
        nativeRemoveARPlane(mNativeRef, arPlane.mNativeRef);
    }

    private native long nativeCreateARSceneController(long nodeRef);

    private native long nativeCreateARSceneDelegate(long sceneControllerRef);

    private native void nativeDestroyARSceneController(long sceneControllerRef);

    private native void nativeDestroyARSceneDelegate(long delegateRef);

    private native void nativeAddARPlane(long sceneControllerRef, long arPlaneRef);

    private native void nativeUpdateARPlane(long sceneControllerRef, long arPlaneRef);

    private native void nativeRemoveARPlane(long sceneControllerRef, long arPlaneRef);

    // -- ARSceneDelegate --

    private WeakReference<ARSceneDelegate> mARSceneDelegate = null;

    public interface ARSceneDelegate {
        void onTrackingInitialized();
        void onAmbientLightUpdate(float lightIntensity, float colorTemperature);
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

}
