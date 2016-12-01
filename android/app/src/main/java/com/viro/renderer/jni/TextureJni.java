/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;


public class TextureJni {
    protected long mNativeRef;

    public TextureJni(ImageJni px, ImageJni nx, ImageJni ny,
                      ImageJni py, ImageJni pz, ImageJni nz) {
        mNativeRef = nativeCreateCubeTexture(px.mNativeRef, nx.mNativeRef,
                                             py.mNativeRef, ny.mNativeRef,
                                             pz.mNativeRef, nz.mNativeRef);
    }

    public TextureJni(ImageJni image) {
        mNativeRef = nativeCreateImageTexture(image.mNativeRef);
    }



    public void destroy() {
        nativeDestroyTexture(mNativeRef);
    }
    private native long nativeCreateCubeTexture(long px, long nx, long py,
                                                long ny, long pz, long nz);
    private native long nativeCreateImageTexture(long image);
    private native void nativeDestroyTexture(long nativeRef);
}
