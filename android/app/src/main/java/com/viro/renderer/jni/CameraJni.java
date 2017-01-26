/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.CameraJni.java
 * Cpp JNI wrapper      : Camera_JNI.cpp
 * Cpp Object           : VRONodeCamera.cpp
 */
public class CameraJni {
    final protected long mNativeRef;

    public CameraJni() {
        mNativeRef = nativeCreateCamera();
    }

    public void destroy(){
        nativeDestroyCamera(mNativeRef);
    }

    public void addToNode(NodeJni nodeJni) {
        nativeAddToNode(mNativeRef, nodeJni.mNativeRef);
    }

    public void removeFromNode(NodeJni nodeJni) {
        nativeRemoveFromNode(mNativeRef, nodeJni.mNativeRef);
    }

    public void setPosition(float[] position) {
        nativeSetPosition(mNativeRef, position[0], position[1], position[2]);
    }
    public void setRotationType(String rotationType) {
        nativeSetRotationType(mNativeRef, rotationType);
    }
    public void setOrbitFocalPoint(float[] position) {
        nativeSetOrbitFocalPoint(mNativeRef, position[0], position[1], position[2]);
    }

    private native long nativeCreateCamera();
    private native void nativeDestroyCamera(long nativeRef);
    private native void nativeAddToNode(long cameraRef, long nodeRef);
    private native void nativeRemoveFromNode(long cameraRef, long nodeRef);
    private native void nativeSetPosition(long nativeRef, float x, float y, float z);
    private native void nativeSetRotationType(long nativeRef, String rotationType);
    private native void nativeSetOrbitFocalPoint(long nativeRef, float x, float y, float z);
}
