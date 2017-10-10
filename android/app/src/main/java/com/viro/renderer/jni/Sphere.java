/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.component.Video360
 * Java JNI Wrapper     : com.viro.renderer.jni.SphereJni.java
 * Cpp JNI wrapper      : Sphere_JNI.cpp
 * Cpp Object           : VROSphere.cpp
 */
public class Sphere extends BaseGeometry{
    public Sphere(float radius, int widthSegmentCount, int heightSegementCount, boolean facesOutward) {
        mNativeRef = nativeCreateSphere(
                radius,
                widthSegmentCount,
                heightSegementCount,
                facesOutward);
    }

    public void delete(){
        nativeDestroySphere(mNativeRef);
    }

    public void setVideoTexture(VideoTexture texture){
        nativeSetVideoTexture(mNativeRef, texture.mNativeRef);
    }

    /**
     * Native Functions called into JNI
     */
    private native long nativeCreateSphere(float radius, int widthSegmentCount, int heightSegementCount, boolean facesOutward);
    private native void nativeDestroySphere(long nodeReference);
    private native void nativeSetVideoTexture(long nodeReference, long textureRef);

    @Override
    public void attachToNode(Node node) {
        nativeAttachToNode(mNativeRef, node.mNativeRef);
    }
    private native void nativeAttachToNode(long boxReference, long nodeReference);
}
