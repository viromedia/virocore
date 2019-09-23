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
 * CameraIntrinsics define the physical characteristics of the device camera using a "pinhole"
 * model.
 */
public class CameraIntrinsics {
    private float[] mFocalLength;
    private float[] mPrincipalPoint;

    /**
     * Invoked from JNI
     * @hide
     */
    public CameraIntrinsics(float fx, float fy, float cx, float cy) {
        this.mFocalLength = new float[]{fx, fy};
        this.mPrincipalPoint = new float[]{cx, cy};
    }

    /**
     * Get the X and Y focal length of the camera.
     *
     * @return The X and Y focal length in an array { X, Y }.
     */
    public float[] getFocalLength() {
        return mFocalLength;
    }

    /**
     * Get the principal point of the camera.
     *
     * @return The X and Y of the principal point in an array { X, Y }.
     */
    public float[] getPrincipalPoint() {
        return mPrincipalPoint;
    }
}
