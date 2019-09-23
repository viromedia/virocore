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

/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Android Java Object  : com.viromedia.bridge.view.Box.java
 * Java JNI Wrapper     : com.viro.renderer.BoxJni.java
 * Cpp JNI wrapper      : Box_JNI.cpp
 * Cpp Object           : VROBox.cpp
 */
package com.viro.core;

/**
 * Box is a simple shape representing a six-sided geometry, defined by width, height, and length.
 */
public class Box extends Geometry {

    float mWidth, mHeight, mLength;

    /**
     * Construct a new Box with the given width, height, and length.
     *
     * @param width  The width of the box (X dimension).
     * @param height The height of the box (Y dimension).
     * @param length The length of the box (Z dimension).
     */
    public Box(float width, float height, float length) {
        super(false);

        mWidth = width;
        mHeight = height;
        mLength = length;
        mNativeRef = nativeCreateBox(width, height, length);
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
        super.dispose();
    }

    /**
     * Set the width of this Box.
     *
     * @param width The size of the Box in the X dimension.
     */
    public void setWidth(float width) {
        mWidth = width;
        nativeSetWidth(mNativeRef, width);
    }

    /**
     * Get the width of this Box.
     *
     * @return The size of the Box in the X dimension.
     */
    public float getWidth() {
        return mWidth;
    }

    /**
     * Set the height of this Box.
     *
     * @param height The size of the Box in the Y dimension.
     */
    public void setHeight(float height) {
        mHeight = height;
        nativeSetHeight(mNativeRef, height);
    }

    /**
     * Get the height of this Box.
     *
     * @return The size of the Box in the Y dimension.
     */
    public float getHeight() {
        return mHeight;
    }

    /**
     * Set the length of this Box.
     *
     * @param length The size of the Box in the Z dimension.
     */
    public void setLength(float length) {
        mLength = length;
        nativeSetLength(mNativeRef, length);
    }

    /**
     * Get the length of this Box.
     *
     * @return The size of the Box in the Z dimension.
     */
    public float getLength() {
        return mLength;
    }

    private native long nativeCreateBox(float width, float height, float length);
    private native void nativeSetWidth(long boxReference, float width);
    private native void nativeSetHeight(long boxReference, float height);
    private native void nativeSetLength(long boxReference, float length);

}
