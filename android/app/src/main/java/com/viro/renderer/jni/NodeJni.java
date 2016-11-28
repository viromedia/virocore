/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.content.Context;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.NodeJni.java
 * Cpp JNI wrapper      : Node_JNI.cpp
 * Cpp Object           : VRONode.cpp
 */
public class NodeJni {
    protected long mNativeRef;

    public NodeJni(Context context) {
        mNativeRef = nativeCreateNode();
    }

    private void setOpacity(float opacity){
        nativeSetOpacity(mNativeRef, opacity);
    }

    public void removeChildNode(NodeJni childNode){
        nativeRemoveFromParent(childNode.mNativeRef);
    }

    public void addChildNode(NodeJni childNode){
        nativeAddChildNode(mNativeRef, childNode.mNativeRef);
    }

    public void setGeometry(BaseGeometry geometry){
        nativeSetGeometry(mNativeRef, geometry.mNativeRef);
    }

    public void setPosition(float x, float y, float z){
        nativeSetPosition(mNativeRef, x, y, z);
    }

    private native long nativeCreateNode();
    private native void nativeDestroyNode(long nodeReference);
    private native void nativeSetOpacity(long nodeReference, float opacity);
    private native void nativeAddChildNode(long nodeReference, long childNodeReference);
    private native void nativeRemoveFromParent(long nodeReference);
    private native void nativeSetGeometry(long nodeReference, long geometryReference);
    private native void nativeSetPosition(long nodeReference, float x, float y, float z);
}
