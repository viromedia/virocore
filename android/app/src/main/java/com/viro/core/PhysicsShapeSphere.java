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
 * Spherical {@link PhysicsShape}.
 */
public class PhysicsShapeSphere implements PhysicsShape {
    private float mRadius;

    /**
     * Construct a new Sphere physics shape.
     * @param radius The radius of the sphere.
     */
    public PhysicsShapeSphere(float radius) {
        mRadius = radius;
    }

    /**
     * Get the radius of this Sphere.
     *
     * @return The radius.
     */
    public float getRadius() {
        return mRadius;
    }

    /**
     * @hide
     * @return
     */
    @Override
    public String getType() {
        return "sphere";
    }

    /**
     * @hide
     * @return
     */
    @Override
    public float[] getParams() {
        return new float[] { mRadius };
    }
}
