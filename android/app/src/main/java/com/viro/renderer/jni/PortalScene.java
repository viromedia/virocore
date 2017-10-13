package com.viro.renderer.jni;

import java.lang.ref.WeakReference;

/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

public class PortalScene extends Node {
    private static long INVALID_REF = Long.MAX_VALUE;
    protected long mNativeDelegateRef =  INVALID_REF;

    public PortalScene() {
        mNativeRef = nativeCreatePortalScene();
        mNativeDelegateRef = nativeCreatePortalDelegate();
        nativeAttachDelegate(mNativeRef, mNativeDelegateRef);
    }

    public void setPassable(boolean passable) {
        nativeSetPassable(mNativeRef, passable);
    }

    public void setPortalEntrance(Portal portal) {

        nativeSetPortalEntrance(mNativeRef, portal.mNativeRef);
    }

    /**
     * Delegate Callback functions called from JNI
     */
    private WeakReference<PortalDelegate> mDelegate = null;

    public void destroy() {
        super.dispose();
        nativeDestroyPortalScene(mNativeRef);
        mNativeRef = INVALID_REF;

        nativeDestroyPortalDelegate(mNativeDelegateRef);
        mNativeDelegateRef = INVALID_REF;
    }


    public interface PortalDelegate {
        void onPortalEnter();
        void onPortalExit();
    }

    public void setPortalSceneDelegate(PortalDelegate delegate) {
        mDelegate = new WeakReference<PortalDelegate>(delegate);
    }

    public void onPortalEnter() {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != INVALID_REF) {
            mDelegate.get().onPortalEnter();
        }
    }

    public void onPortalExit() {
        if (mDelegate != null && mDelegate.get() != null && mNativeRef != INVALID_REF) {
            mDelegate.get().onPortalExit();
        }
    }

    private native long nativeCreatePortalScene();
    private native long nativeCreatePortalDelegate();
    private native long nativeDestroyPortalScene(long nativeRef);
    private native long nativeDestroyPortalDelegate(long delegateRef);

    private native long nativeAttachDelegate(long nativeRef, long delegateRef);
    private native void nativeSetPassable(long nativeRef, boolean passable);
    private native void nativeSetPortalEntrance(long nativeRef, long portalNativeRef);
}
