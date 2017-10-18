/*
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class ARPlane extends ARNode {

    public ARPlane(float minWidth, float minHeight) {
        super();
        setNativeRef(nativeCreateARPlane(minWidth, minHeight));
    }

    public void destroy() {
        super.destroy();
        nativeDestroyARPlane(mNativeRef);
    }

    @Override
    protected long createNativeARNodeDelegate(long nativeRef) {
        return nativeCreateARPlaneDelegate(nativeRef);
    }

    @Override
    void destroyNativeARNodeDelegate(long delegateRef) {
        nativeDestroyARPlaneDelegate(delegateRef);
    }

    public void setMinWidth(float minWidth) {
        nativeSetMinWidth(mNativeRef, minWidth);
    }

    public void setMinHeight(float minHeight) {
        nativeSetMinHeight(mNativeRef, minHeight);
    }

    public void setAnchorId(String anchorId) {
        nativeSetAnchorId(mNativeRef, anchorId);
    }

    public void setPauseUpdates(boolean pauseUpdates) {
        nativeSetPauseUpdates(mNativeRef, pauseUpdates);
    }

    private native long nativeCreateARPlane(float minWidth, float minHeight);

    private native void nativeDestroyARPlane(long nativeRef);

    private native void nativeSetMinWidth(long nativeRef, float minWidth);

    private native void nativeSetMinHeight(long nativeRef, float minHeight);

    private native void nativeSetAnchorId(long nativeRef, String anchorId);

    private native void nativeSetPauseUpdates(long nativeRef, boolean pauseUpdates);

    private native long nativeCreateARPlaneDelegate(long nativeRef);

    private native void nativeDestroyARPlaneDelegate(long delegateRef);
}
