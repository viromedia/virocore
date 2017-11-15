/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

package com.viro.core;

/**
 * Box physics shape.
 */
public class PhysicsShapeBox implements PhysicsShape {
    private float mWidth, mHeight, mLength;

    /**
     * Construct a new Box physics shape.
     *
     * @param width  The width of the Box (X).
     * @param height The height of the Box (Y).
     * @param length The length of the Box (Z).
     */
    public PhysicsShapeBox(float width, float height, float length) {
        mWidth = width;
        mHeight = height;
        mLength = length;
    }

    /**
     * Get the width of this Box.
     *
     * @return The width.
     */
    public float getWidth() {
        return mWidth;
    }

    /**
     * Get the height of this Box.
     *
     * @return The height.
     */
    public float getHeight() {
        return mHeight;
    }

    /**
     * Get the length of this Box.
     *
     * @return The length.
     */
    public float getLength() {
        return mLength;
    }

    /**
     * @hide
     * @return
     */
    @Override
    public String getType() {
        return "box";
    }

    /**
     * @hide
     * @return
     */
    @Override
    public float[] getParams() {
        return new float[] { mWidth, mHeight, mLength };
    }
}