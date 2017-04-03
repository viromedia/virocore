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

    public TextureJni(ImageJni image, TextureFormat format, boolean mipmap) {
        mNativeRef = nativeCreateImageTexture(image.mNativeRef, format.getID(), mipmap, null);
    }

    public TextureJni(ImageJni image, TextureFormat format, boolean mipmap, String stereoMode) {
        mNativeRef = nativeCreateImageTexture(image.mNativeRef, format.getID(), mipmap, stereoMode);
    }

    public void destroy() {
        nativeDestroyTexture(mNativeRef);
    }
    private native long nativeCreateCubeTexture(long px, long nx, long py,
                                                long ny, long pz, long nz,
                                                String format);
    private native long nativeCreateImageTexture(long image, String format, boolean mipmap, String stereoMode);
    private native void nativeDestroyTexture(long nativeRef);
}
