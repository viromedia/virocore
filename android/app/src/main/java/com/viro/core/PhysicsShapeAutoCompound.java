/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
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
        return "compound";
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