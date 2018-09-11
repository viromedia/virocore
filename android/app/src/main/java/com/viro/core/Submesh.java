/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import java.util.List;

/**
 * Submesh defines how the vertices of a {@link Geometry} are connected together to form a 3D object.
 * Each individual Submesh is an interconnected set of triangles, with each triangle composed
 * from three vertices of the parent Geometry. Submeshes are assigned to a Geometry via
 * {@link Geometry#setSubmeshes(List)}.
 */
public class Submesh {

    long mNativeRef;
    private List<Integer> mTriangles;

    /**
     * Create a new Submesh.
     */
    public Submesh() {
        mNativeRef = nativeCreateGeometryElement();
    }

    /**
     * Set the triangles that define the faces of this submesh. Each integer in this list
     * refers to a vertex in the parent Geometry's vertices list (e.g. the vertices that were
     * assigned to the {@link Geometry} via {@link Geometry#setVertices(List)}. Every three
     * integers in this list defines a triangle. For example, six entries in the triangles list
     * defines two triangles.
     *
     * @param triangles The triangles index list.
     */
    public void setTriangleIndices(List<Integer> triangles) {
        mTriangles = triangles;

        int[] indices = new int[triangles.size()];
        for (int i = 0; i < triangles.size(); i++) {
            indices[i] = triangles.get(i);
        }
        nativeSetTriangleIndices(mNativeRef, indices);
    }

    /**
     * Get the triangle indices that define the faces of this submesh. See {@link
     * #setTriangleIndices(List)} for more information.
     *
     * @return The triangle indices.
     */
    public List<Integer> getTriangleIndices() {
        return mTriangles;
    }

    private native long nativeCreateGeometryElement();
    private native void nativeSetTriangleIndices(long nativeRef, int[] indices);

    /**
     * Retrieve a builder for creating {@link Submesh} objects.
     */
    public static SubmeshBuilder builder() {
        return new SubmeshBuilder();
    }

    /**
     * Builder for creating {@link Submesh} objects.
     */
    public static class SubmeshBuilder {

        private List<Integer> mTriangleIndices;

        /**
         * Set the {@link Submesh}es to be used for building this {@link Geometry}.
         *
         * @return This builder.
         */
        public SubmeshBuilder triangleIndices(List<Integer> triangleIndices) {
            this.mTriangleIndices = triangleIndices;
            return this;
        }

        /**
         * Return the built {@link Submesh}.
         *
         * @return The built Submesh.
         */
        public Submesh build() {
            Submesh submesh = new Submesh();
            submesh.setTriangleIndices(mTriangleIndices);
            return submesh;
        }
    }
}
