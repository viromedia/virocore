/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import java.util.List;

/**
 * Geometry is the top-level class for any displayable 3D model with {@link Material} objects that
 * define its appearance. Geometry objects can be simple shapes (e.g. {@link Box} or {@link
 * Sphere}), full 3D models (e.g. {@link Object3D}), or UI components (e.g. {@link Text}).
 */
public class Geometry {

    protected long mNativeRef;
    private List<Material> mMaterials;
    private List<Submesh> mSubmeshes;

    /**
     * Construct a new Geometry.
     */
    public Geometry() {
        mNativeRef = nativeCreateGeometry();
    }

    /**
     * Construct a Geometry to wrap the given native reference.
     *
     * @param nativeRef The native reference.
     * @hide
     */
    Geometry(long nativeRef) {
        mNativeRef = nativeRef;
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this Box.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyGeometry(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Get the {@link Material} objects used by this Geometry.
     *
     * @return List containing each {@link Material} used by this Geometry.
     */
    public List<Material> getMaterials() {
        return mMaterials;
    }

    /**
     * Set the {@link Material} objects that define the Geometry's appearance.
     * <p>
     * Materials consist of a variety of visual properties, including colors and textures, that
     * define how the Geometry interacts with the Lights in a Scene to render each pixel.
     * <p>
     * Geometries may contain multiple Materials; how those Materials map to the surfaces of a
     * Geometry is determined by each individual Geometry.
     *
     * @param materials The list of Materials to use for this Geometry.
     */
    public void setMaterials(List<Material> materials) {
        mMaterials = materials;

        // Create list of longs (refs) to all the materials. If any
        // material has already been destroyed, return false
        long[] materialRefs = new long[materials.size()];
        for (int i = 0; i < materials.size(); i++) {
            materialRefs[i] = materials.get(i).mNativeRef;
            if (materialRefs[i] == 0) {
                return;
            }
        }
        nativeSetMaterials(mNativeRef, materialRefs);
    }

    /**
     * Get the {@link Submesh}es used by this Geometry. Submeshes define how to
     * connect the vertices in this Geometry to form a 3D object.
     *
     * @return
     */
    public List<Submesh> getSubmeshes() {
        return mSubmeshes;
    }

    /**
     * Set the {@link Submesh}es for this Geometry. Submeshes define how to connect the vertices in
     * this Geometry to form a 3D object. Each Submesh contains a triangle definition that indicates
     * how to join the Geometry's vertices into a set of triangles that will form the surfaces of
     * this model.
     * <p>
     * Submeshes are also tied to {@link Material}s: Submesh <tt>i</tt> uses Material <tt>i</tt>. If
     * there are fewer materials than submeshes, then Submesh <tt>i</tt> will use Material <tt>i %
     * materials.size()</tt>.
     *
     * @param submeshes The {@link Submesh}es that will define the surfaces of this Geometry.
     */
    public void setSubmeshes(List<Submesh> submeshes) {
        mSubmeshes = submeshes;

        long[] elementRefs = new long[submeshes.size()];
        for (int i = 0; i < submeshes.size(); i++) {
            elementRefs[i] = submeshes.get(i).mNativeRef;
            if (elementRefs[i] == 0) {
                return;
            }
        }
        nativeSetGeometryElements(mNativeRef, elementRefs);
    }

    /**
     * Set the vertices of this Geometry. The vertices define the physical shape of the
     * Geometry: each is an X, Y, Z coordinate. Vertices are grouped together to form triangles,
     * which together create the surface of the Geometry. The grouping together of the vertices
     * into triangles is defined by the {@link Submesh}es.
     *
     * @param vertices The vertices to use for this Geometry.
     */
    public void setVertices(List<Vector> vertices) {
        float[] vertexData = new float[vertices.size() * 3];
        for (int i = 0; i < vertices.size(); i++) {
            vertexData[i * 3 + 0] = vertices.get(i).x;
            vertexData[i * 3 + 1] = vertices.get(i).y;
            vertexData[i * 3 + 2] = vertices.get(i).z;
        }

        nativeSetVertices(mNativeRef, vertexData);
    }

    /**
     * Set the texture coordinates used by this Geometry. Texture coordinates define how the
     * 2D textures used by the Geometry are mapped onto its 3D surfaces. Like vertices, texture
     * coordinates are grouped together by triangles. The grouping into triangles is defined by
     * the {@link Submesh}es.
     *
     * @param textureCoordinates The texture coordinates used by this Geometry. Each {@link Vector}
     *                           uses X for the texture U coordinate, and the Y for the texture V
     *                           coordinate. The Z coordinate is ignored.
     */
    public void setTextureCoordinates(List<Vector> textureCoordinates) {
        float[] texcoordData = new float[textureCoordinates.size() * 2];
        for (int i = 0; i < textureCoordinates.size(); i++) {
            texcoordData[i * 2 + 0] = textureCoordinates.get(i).x;
            texcoordData[i * 2 + 1] = textureCoordinates.get(i).y;
        }

        nativeSetTextureCoordinates(mNativeRef, texcoordData);
    }

    /**
     * Set the normal vectors used by this Geometry. Normal vectors define the orientation of the
     * surfaces of this Geometry, which determines how each surface responds to light and other
     * environmental factors. Like vertices and texture coordinates, normals are grouped into
     * triangles by the {@link Submesh}es. When rendering a triangle, the normal at each
     * point is interpolated from the normals defined by each vertex of the triangle.
     *
     * @param normals The normal vectors to use for this Geometry.
     */
    public void setNormals(List<Vector> normals) {
        float[] normalData = new float[normals.size() * 3];
        for (int i = 0; i < normals.size(); i++) {
            normalData[i * 3 + 0] = normals.get(i).x;
            normalData[i * 3 + 1] = normals.get(i).y;
            normalData[i * 3 + 2] = normals.get(i).z;
        }

        nativeSetNormals(mNativeRef, normalData);
    }

    /**
     * @hide
     * Create list of longs (refs) to all the materials. If any
     * material has already been destroyed, return false
     */
    //#IFDEF 'viro_react'
    public void copyAndSetMaterials(List<Material> materials) {
        long[] materialRefs = new long[materials.size()];
        for (int i = 0; i < materials.size(); i++) {
            materialRefs[i] = materials.get(i).mNativeRef;
            if (materialRefs[i] == 0) {
                return;
            }
        }
        nativeCopyAndSetMaterials(mNativeRef, materialRefs);
    }
    //#ENDIF

    private native long nativeCreateGeometry();
    private native void nativeDestroyGeometry(long nativeRef);
    private native void nativeSetMaterials(long nativeRef, long[] materials);
    private native void nativeCopyAndSetMaterials(long nativeRef, long[] materials);
    private native void nativeSetGeometryElements(long nativeRef, long[] elements);
    private native void nativeSetVertices(long nativeRef, float[] vertices);
    private native void nativeSetNormals(long nativeRef, float[] vertices);
    private native void nativeSetTextureCoordinates(long nativeRef, float[] vertices);

    /**
     * Retrieve a builder for creating {@link Geometry} objects.
     */
    public static GeometryBuilder geometryBuilder() {
        return new GeometryBuilder();
    }

    /**
     * Builder for creating {@link Geometry} objects.
     */
    public static class GeometryBuilder {

        private List<Material> mMaterials;
        private List<Submesh> mSubmeshes;

        /**
         * Set the {@link Submesh}es to be used for building this {@link Geometry}.
         *
         * @return This builder.
         */
        public GeometryBuilder submeshes(List<Submesh> submeshes) {
            this.mSubmeshes = submeshes;
            return this;
        }

        /**
         * Set the {@link Material}s to be used for building this {@link Geometry}.
         *
         * @return This builder.
         */
        public GeometryBuilder materials(List<Material> materials) {
            this.mMaterials = materials;
            return this;
        }

        /**
         * Return the built {@link Geometry}.
         *
         * @return The built Geometry.
         */
        public Geometry build() {
            Geometry geo = new Geometry();
            geo.setMaterials(mMaterials);
            geo.setSubmeshes(mSubmeshes);
            return geo;
        }
    }

}
