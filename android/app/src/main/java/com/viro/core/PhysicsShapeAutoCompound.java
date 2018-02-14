/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

package com.viro.core;

/**
 * AutoCompound shapes are automatically generated from a Node by approximating its geometry
 * as a composite of bounding boxes.
 */
public class PhysicsShapeAutoCompound implements PhysicsShape {

    /**
     * Construct a new AutoCompound Shape.
     */
    public PhysicsShapeAutoCompound() {

    }

    /**
     * @return
     * @hide
     */
    @Override
    public String getType() {
        return "Compound";
    }

    /**
     * @return
     * @hide
     */
    @Override
    public float[] getParams() {
        return new float[0];
    }
}