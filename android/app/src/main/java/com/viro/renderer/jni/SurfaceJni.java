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
public class SurfaceJni extends BaseGeometry {

    public SurfaceJni(float width, float height, float u0, float v0, float u1, float v1) {
        mNativeRef = nativeCreateSurface(width, height, u0, v0, u1, v1);
    }

    public SurfaceJni(float width, float height, float u0, float v0, float u1, float v1,
                      SurfaceJni oldSurface) {
        mNativeRef = nativeCreateSurfaceFromSurface(width, height, u0, v0, u1, v1,
                oldSurface.mNativeRef);
    }

    public void destroy(){
        nativeDestroySurface(mNativeRef);
    }

    public void setVideoTexture(VideoTextureJni texture){
        nativeSetVideoTexture(mNativeRef, texture.mNativeRef);
    }

    public void setImageTexture(TextureJni texture) {
        nativeSetImageTexture(mNativeRef, texture.mNativeRef);
    }

    public void setMaterial(MaterialJni material) {
        nativeSetMaterial(mNativeRef, material.mNativeRef);
    }

    public void setWidth(float width) {
        nativeSetWidth(mNativeRef, width);
    }

    public void setHeight(float height) {
        nativeSetHeight(mNativeRef, height);
    }

    public void clearMaterial() {
        nativeClearMaterial(mNativeRef);
    }

    /**
     * Native Functions called into JNI
     */
    private native long nativeCreateSurface(float width, float height,
                                            float u0, float v0, float u1, float v1);
    private native long nativeCreateSurfaceFromSurface(float width, float height,
                                                       float u0, float v0, float u1, float v1,
                                                       long oldSurfaceRef);

    private native void nativeDestroySurface(long surfaceRef);
    private native void nativeSetWidth(long surfaceRef, float width);
    private native void nativeSetHeight(long surfaceRef, float height);
    private native void nativeSetVideoTexture(long surfaceRef, long textureRef);
    private native void nativeSetImageTexture(long surfaceRef, long textureRef);
    private native void nativeSetMaterial(long surfaceRef, long materialRef);
    private native void nativeClearMaterial(long surfaceRef);

    @Override
    public void attachToNode(NodeJni node) {
        nativeAttachToNode(mNativeRef, node.mNativeRef);
    }
    private native void nativeAttachToNode(long surfaceRef, long nodeReference);
}
