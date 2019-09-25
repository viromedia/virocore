//
//  Copyright (c) 2018-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.core;

/**
 * BoundingBox defines an axis-aligned 6-sided bounding volume.
 */
public class BoundingBox {
    /**
     * The minimum X value.
     */
    public float minX;

    /**
     * The maximum X value.
     */
    public float maxX;

    /**
     * The minimum Y value.
     */
    public float minY;

    /**
     * The maximum Y value.
     */
    public float maxY;

    /**
     * The minimum Z value.
     */
    public float minZ;

    /**
     * The maximum Z value.
     */
    public float maxZ;

    /**
     * Constructs a new BoundingBox with the given bounds on each axis.
     *
     * @param minX The minimum X value.
     * @param maxX The maximum X value.
     * @param minY The minimum Y value.
     * @param maxY The maximum Y value.
     * @param minZ The minimum Z value.
     * @param maxZ The maximum Z value.
     */
    public BoundingBox(float minX, float maxX, float minY, float maxY, float minZ, float maxZ) {
        this.minX = minX;
        this.maxX = maxX;
        this.minY = minY;
        this.maxY = maxY;
        this.minZ = minZ;
        this.maxZ = maxZ;
    }

    /**
     * Construct a new bounding box given a float array containing 6 elements representing
     * the bounds.
     *
     * @param bounds The bounds on each axis, in order [minX, maxX, minY, maxY, minZ, maxZ].
     */
    public BoundingBox(float[] bounds) {
        this(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
    }

    @Override
    public String toString() {
        return "[minX: " + minX + ", maxX: " + maxX +
              ", minY: " + minY + ", maxY: " + maxY +
              ", minZ: " + minZ + ", maxZ: " + maxZ + "]";
    }
}
