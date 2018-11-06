/*
 * Copyright (c) 2017-present, Viro Media, Inc.
 * All rights reserved.
 */

/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Cpp JNI wrapper      : Quad_JNI.cpp
 * Cpp Object           : VROSurface.cpp
 */
package com.viro.core;

import java.util.Arrays;

/**
 * Quad represents a one-sided plane defined by a width and height.
 */
public class Quad extends Geometry {

    private float mWidth, mHeight;

    /**
     * Construct a new Quad with the given width and height.
     *
     * @param width  The extent of the Quad along its horizontal axis (X).
     * @param height The extent of the Quad along its vertical axis (Y).
     */
    public Quad(float width, float height) {
        this(width, height, 0, 0, 1, 1);
    }


    /**
     * Construct a new Quad with custom texture coordinates. Texture coordinates specify how to
     * tile a {@link Texture} across the Quad's geometry.
     * <p>
     * Texture coordinates are represented on 2D U and V axes (essentially the X and Y axes of the
     * image). The left edge of a texture is U = 0.0 and the right edge of the texture is U = 1.0.
     * Similarly, the top edge of a texture is V=0.0 and the bottom edge of the texture is V=1.0.
     * <p>
     * Specifying greater than 1.0 on either the U or V axis will cause the tile to repeat itself or
     * clamp, depending on the Texture's {@link Texture#setWrapS(Texture.WrapMode)}. Specifying less
     * than 1.0 on the U or V axis will render that texture partially over the entire quad.
     * <p>
     * For example, specifying u0,v0 as (0,0) and (u1,v2) as (2,2) will tile the Texture twice over
     * the width and height of the Quad, effectively displaying 4 textures on the Quad.
     * Alternatively, setting (u1,v1) as (0.5, 0.5) will display a quarter of the texture over the
     * entire quad.
     * <p>
     *
     * @param width  The extent of the Quad along its horizontal axis (X).
     * @param height The extent of the Quad along its vertical axis (Y).
     * @param u0     The texture coordinate that specifies the start {@link Texture} left edge.
     * @param v0     The texture coordinate that specifies the start {@link Texture} top edge.
     * @param u1     The texture coordinate that specifies the end {@link Texture} left edge.
     * @param v1     The texture coordinate that specifies the end {@link Texture} top edge.
     */
    public Quad(float width, float height, float u0, float v0, float u1, float v1) {
        super(false);
        mWidth = width;
        mHeight = height;
        mNativeRef = nativeCreateQuad(width, height, u0, v0, u1, v1);
    }

    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public Quad(float width, float height, float u0, float v0, float u1, float v1,
                Quad oldQuad) {
        super(false);
        mNativeRef = nativeCreateQuadFromQuad(width, height, u0, v0, u1, v1,
                oldQuad.mNativeRef);
    }
    //#ENDIF

    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this Quad.
     */
    public void dispose(){
        super.dispose();
    }

    /**
     * Set the width of this Quad.
     *
     * @param width The extent of the Quad along its horizontal axis.
     */
    public void setWidth(float width) {
        mWidth = width;
        nativeSetWidth(mNativeRef, width);
    }

    /**
     * Get the width of this Quad.
     *
     * @return The extent of this Quad along its horizontal axis.
     */
    public float getWidth() {
        return mWidth;
    }

    /**
     * Set the height of this Quad.
     *
     * @param height The extent of the Quad along its vertical axis.
     */
    public void setHeight(float height) {
        mHeight = height;
        nativeSetHeight(mNativeRef, height);
    }

    /**
     * Get the height of this Quad.
     *
     * @return The extent of this Quad along its vertical axis.
     */
    public float getHeight() {
        return mHeight;
    }

    private native long nativeCreateQuad(float width, float height, float u0, float v0, float u1, float v1);
    private native long nativeCreateQuadFromQuad(float width, float height,
                                                       float u0, float v0, float u1, float v1,
                                                       long oldSurfaceRef);
    private native void nativeSetWidth(long quadRef, float width);
    private native void nativeSetHeight(long quadRef, float height);
    private native void nativeSetVideoTexture(long quadRef, long textureRef);
    private native void nativeSetImageTexture(long quadRef, long textureRef);
    private native void nativeClearMaterial(long quadRef);

    /**
     * @hide
     * @param texture
     */
    //#IFDEF 'viro_react'
    public void setVideoTexture(VideoTexture texture){
        nativeSetVideoTexture(mNativeRef, texture.mNativeRef);
    }
    //#ENDIF
    /**
     * @hide
     * @param texture
     */
    //#IFDEF 'viro_react'
    public void setImageTexture(Texture texture) {
        nativeSetImageTexture(mNativeRef, texture.mNativeRef);
    }
    //#ENDIF
    /**
     * @hide
     * @param material
     */
    //#IFDEF 'viro_react'
    public void setMaterial(Material material) {
        super.setMaterials(Arrays.asList(material));
    }
    //#ENDIF
    /**
     * @hide
     */
    //#IFDEF 'viro_react'
    public void clearMaterial() {
        nativeClearMaterial(mNativeRef);
    }
    //#ENDIF
}
