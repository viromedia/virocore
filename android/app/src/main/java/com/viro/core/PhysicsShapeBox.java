//
//  Copyright (c) 2017-present, ViroMedia, Inc.
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
        return "Box";
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