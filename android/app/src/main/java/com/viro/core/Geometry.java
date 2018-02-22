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

    /**
     * Construct a new Geometry.
     * @hide
     */
    Geometry() {}

    /**
     * Construct a Geometry to wrap the given native reference.
     *
     * @param nativeRef The native reference.
     * @hide
     */
    Geometry(long nativeRef) {
        mNativeRef = nativeRef;
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

    private native void nativeSetMaterials(long nativeRef, long[] materials);
    private native void nativeCopyAndSetMaterials(long nativeRef, long[] materials);

}
