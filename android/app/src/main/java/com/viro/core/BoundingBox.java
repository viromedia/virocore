/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * BoundingBox defines the axis-aligned box that encompasses a node's Geometry.
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
     * Constructs a new bounding box with the given bounds.
     *
     * @param minX - The minimum X value.
     * @param maxX - The maximum X value.
     * @param minY - The minimum Y value.
     * @param maxY - The maximum Y value.
     * @param minZ - The minimum Z value.
     * @param maxZ - The maximum Z value.
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
     * @param bounds
     */
    public BoundingBox(float[] bounds) {
        this(bounds[0], bounds[1], bounds[2], bounds[3], bounds[4], bounds[5]);
    }
}
