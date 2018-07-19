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
 * Polygon represents a one-sided plane whose boundary is defined by a list of vertices.
 */
public class Polygon extends Geometry {

    /**
     * Construct a flat polygon with the given perimeter. The perimeter is specified as a list of 2D
     * points on the XY plane. Both convex and concave polygonal shapes are supported.
     * <p>
     * Texture coordinates are represented on 2D U and V axes (essentially the X and Y axes of the
     * texture). The left edge of the texture is U = 0.0 and the right edge of the texture is U =
     * 1.0. Similarly, the top edge of a texture is V = 0.0 and the bottom edge of the texture is V
     * = 1.0.
     * <p>
     * Specifying greater than 1.0 on either the U or V axis will cause the texture to either tile
     * and repeat itself or clamp, depending on the Texture's {@link Texture#setWrapS(Texture.WrapMode)}.
     * Specifying less than 1.0 on the U or V axis will render that texture partially over the
     * surface.
     * <p>
     * For example, specifying u0,v0 as (0,0) and (u1,v2) as (2,2) will tile the Texture twice over
     * the width and height of the polygon, effectively displaying 4 textures on the polygon.
     * Alternatively, setting (u1,v1) as (0.5, 0.5) will display a quarter of the texture over the
     * polygon.
     * <p>
     *
     * @param vertices The list of boundary vertex positions in local model space.
     * @param u0       The texture coordinate that specifies the start {@link Texture} left edge.
     * @param v0       The texture coordinate that specifies the start {@link Texture} top edge.
     * @param u1       The texture coordinate that specifies the end {@link Texture} left edge.
     * @param v1       The texture coordinate that specifies the end {@link Texture} top edge.
     */
    public Polygon(List<Vector> vertices, float u0, float v0, float u1, float v1) {
        float[][] data = new float[vertices.size()][2];
        for (int i = 0; i < vertices.size(); i ++) {
            Vector point = vertices.get(i);
            data[i] = new float[] { point.x, point.y };
        }

        mNativeRef = nativeCreatePolygon(data, null, u0, v0, u1, v1);
    }

    /**
     * Construct a flat polygon with the given perimeter and holes. The perimeter is specified as a
     * list of 2D points on the XY plane, and the holes are each similarly represented as a list of 2D points.
     * Both convex and concave polygonal shapes are supported.
     * <p>
     * Texture coordinates are represented on 2D U and V axes (essentially the X and Y axes of the
     * texture). The left edge of the texture is U = 0.0 and the right edge of the texture is U =
     * 1.0. Similarly, the top edge of a texture is V = 0.0 and the bottom edge of the texture is V
     * = 1.0.
     * <p>
     * Specifying greater than 1.0 on either the U or V axis will cause the texture to either tile
     * and repeat itself or clamp, depending on the Texture's {@link Texture#setWrapS(Texture.WrapMode)}.
     * Specifying less than 1.0 on the U or V axis will render that texture partially over the
     * surface.
     * <p>
     * For example, specifying u0,v0 as (0,0) and (u1,v2) as (2,2) will tile the Texture twice over
     * the width and height of the polygon, effectively displaying 4 textures on the polygon.
     * Alternatively, setting (u1,v1) as (0.5, 0.5) will display a quarter of the texture over the
     * polygon.
     * <p>
     *
     * @param vertices The list of boundary (perimeter) vertex positions in local model space.
     * @param holes    The boundary of each hole in the polygon. Each hole is given as a separate List of
     *                 Vectors.
     * @param u0       The texture coordinate that specifies the start {@link Texture} left edge.
     * @param v0       The texture coordinate that specifies the start {@link Texture} top edge.
     * @param u1       The texture coordinate that specifies the end {@link Texture} left edge.
     * @param v1       The texture coordinate that specifies the end {@link Texture} top edge.
     */
    public Polygon(List<Vector> vertices, List<List<Vector>> holes, float u0, float v0, float u1, float v1) {
        float[][] path = new float[vertices.size()][2];
        for (int i = 0; i < vertices.size(); i ++) {
            Vector point = vertices.get(i);
            path[i] = new float[] { point.x, point.y };
        }

        if (holes != null && holes.size() > 0) {
            float[][][] holeData = new float[holes.size()][][];

            for (int i = 0; i < holes.size(); i++) {
                List<Vector> hole = holes.get(i);
                holeData[i] = new float[hole.size()][2];

                for (int j = 0; j < hole.size(); j++) {
                    Vector point = hole.get(j);
                    holeData[i][j] = new float[] { point.x, point.y };
                }
            }
            mNativeRef = nativeCreatePolygon(path, holeData, u0, v1, u1, v1);
        }
        else {
            mNativeRef = nativeCreatePolygon(path, null, u0, v0, u1, v1);
        }
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
            nativeDestroyPolygon(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreatePolygon(float[][] path, float[][][] holes, float u0, float v0, float u1, float v1);
    private native void nativeDestroyPolygon(long polygonRef);
}