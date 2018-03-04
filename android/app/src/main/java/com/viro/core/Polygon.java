/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */

/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Cpp JNI wrapper      : Polygon_JNI.cpp
 * Cpp Object           : VROPolygon.cpp
 */
package com.viro.core;
import java.util.List;

/**
 * A Polygon represents a one-sided plane defined by a list of vertices.
 */
public class Polygon extends Geometry {

    /**
     * Given a list of 2D points in xy coordinates, construct a flat polygon shape. This shape is
     * constructed in the the order of the provided points in the list of vertices.
     * <p>
     * Texture coordinates are represented on 2D U and V axes (essentially the X and Y axes of the
     * image). The left edge of a texture is U = 0.0 and the right edge of the texture is U = 1.0.
     * Similarly, the top edge of a texture is V=0.0 and the bottom edge of the texture is V=1.0.
     * <p>
     * Specifying greater than 1.0 on either the U or V axis will cause the tile to repeat itself or
     * clamp, depending on the Texture's {@link Texture#setWrapS(Texture.WrapMode)}. Specifying less
     * than 1.0 on the U or V axis will render that texture partially over the entire surface.
     * <p>
     * For example, specifying u0,v0 as (0,0) and (u1,v2) as (2,2) will tile the Texture twice over
     * the width and height of the Surface, effectively displaying 4 textures on the Surface.
     * Alternatively, setting (u1,v1) as (0.5, 0.5) will display a quarter of the texture over the
     * entire surface.
     * <p>
     * @param vertices The list of vertex positions, in local model space, that when rendered
     *                         in order outlines the edges of this polygon shape.
     * @param u0     The texture coordinate that specifies the start {@link Texture} left edge.
     * @param v0     The texture coordinate that specifies the start {@link Texture} top edge.
     * @param u1     The texture coordinate that specifies the end {@link Texture} left edge.
     * @param v1     The texture coordinate that specifies the end {@link Texture} top edge.
     */
    public Polygon(List<Vector> vertices, float u0, float v0, float u1, float v1) {
        float[][] data = new float[vertices.size()][2];
        for (int i = 0; i < vertices.size(); i ++) {
            Vector point = vertices.get(i);
            float[] jPoint = new float[2];
            jPoint[0] = point.x;
            jPoint[1] = point.y;
            data[i] = jPoint;
        }

        mNativeRef = nativeCreateSurface(data, u0, v0, u1, v1);
    }

    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this Polygon.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroySurface(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreateSurface(float[][] data, float u0, float v0, float u1, float v1);
    private native void nativeDestroySurface(long surfaceRef);
}