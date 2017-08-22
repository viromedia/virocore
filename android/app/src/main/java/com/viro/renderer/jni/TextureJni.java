/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public class TextureJni {
    protected long mNativeRef;

    public TextureJni(ImageJni px, ImageJni nx, ImageJni py,
                      ImageJni ny, ImageJni pz, ImageJni nz,
                      TextureFormat format) {
        mNativeRef = nativeCreateCubeTexture(px.mNativeRef, nx.mNativeRef,
                                             py.mNativeRef, ny.mNativeRef,
                                             pz.mNativeRef, nz.mNativeRef,
                                             format.getID());
    }

    public TextureJni(ImageJni image, TextureFormat format, boolean sRGB, boolean mipmap) {
        mNativeRef = nativeCreateImageTexture(image.mNativeRef, format.getID(), sRGB, mipmap, null);
    }

    public TextureJni(ImageJni image, TextureFormat format, boolean sRGB, boolean mipmap, String stereoMode) {
        mNativeRef = nativeCreateImageTexture(image.mNativeRef, format.getID(), sRGB, mipmap, stereoMode);
    }

    public void setWrapS(String wrapS) {
        nativeSetWrapS(mNativeRef, wrapS);
    }

    public void setWrapT(String wrapT) {
        nativeSetWrapT(mNativeRef, wrapT);
    }

    public void setMinificationFilter(String minificationFilter) {
        nativeSetMinificationFilter(mNativeRef, minificationFilter);
    }

    public void setMagnificationFilter(String magnificationFilter) {
        nativeSetMagnificationFilter(mNativeRef, magnificationFilter);
    }

    public void setMipFilter(String mipFilter) {
        nativeSetMipFilter(mNativeRef, mipFilter);
    }

    public void destroy() {
        nativeDestroyTexture(mNativeRef);
    }
    private native long nativeCreateCubeTexture(long px, long nx, long py,
                                                long ny, long pz, long nz,
                                                String format);
    private native long nativeCreateImageTexture(long image, String format, boolean sRGB, boolean mipmap, String stereoMode);
    private native void nativeSetWrapS(long nativeRef, String wrapS);
    private native void nativeSetWrapT(long nativeRef, String wrapT);
    private native void nativeSetMinificationFilter(long nativeRef, String minificationFilter);
    private native void nativeSetMagnificationFilter(long nativeRef, String magnificationFilter);
    private native void nativeSetMipFilter(long nativeRef, String mipFilter);
    private native void nativeDestroyTexture(long nativeRef);
}
