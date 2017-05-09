/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


public class PolylineJni extends BaseGeometry {
    public PolylineJni(float[][] points, float width) {
        mNativeRef = nativeCreatePolyline(points, width);
    }

    public void destroy() {
        nativeDestroyPolyline(mNativeRef);
    }

    @Override
    public void attachToNode(NodeJni node) {
        nativeAttachToNode(mNativeRef, node.mNativeRef);
    }

    public void setThickness(float thickness) {
        nativeSetThickness(mNativeRef, thickness);
    }

    private native long nativeCreatePolyline(float[][] points, float width);
    private native void nativeDestroyPolyline(long lineReference);
    private native void nativeAttachToNode(long lineReference, long nodeReference);
    private native void nativeSetThickness(long lineReference, float thickness);
}
