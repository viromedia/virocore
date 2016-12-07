/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.jni.SurfaceJni.java
 * Cpp JNI wrapper      : Surface_JNI.cpp
 * Cpp Object           : VROSurface.cpp
 */
public class SurfaceJni extends BaseGeometry{
    public SurfaceJni(float width, float height) {
        mNativeRef = nativeCreateSurface(
                width, height);
    }

    public void delete(){
        nativeDestroySurface(mNativeRef);
    }

    public void setVideoTexture(VideoTextureJni texture){
        nativeSetVideoTexture(mNativeRef, texture.mNativeRef);
    }

    /**
     * Native Functions called into JNI
     */
    private native long nativeCreateSurface(float width, float height);
    private native void nativeDestroySurface(long nodeReference);

    private native void nativeSetVideoTexture(long nodeReference, long textureRef);

    @Override
    public void attachToNode(NodeJni node) {
        nativeAttachToNode(mNativeRef, node.mNativeRef);
    }
    private native void nativeAttachToNode(long boxReference, long nodeReference);
}
