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

import java.util.ArrayList;
import java.util.List;

/**
 * Polyline defines a multi-point line.
 */
public class Polyline extends Geometry {

    private float mThickness;
    private List<Vector> mPoints;

    /**
     * Construct a new Polyline with no points. Points may be added via {@link #setPoints(List)} or
     * {@link #appendPoint(Vector)}.
     *
     * @param thickness The thickness of the Polyline.
     */
    public Polyline(float thickness) {
        super(false);
        mPoints = new ArrayList<Vector>();
        mNativeRef = nativeCreatePolylineEmpty(thickness);
    }

    /**
     * Construct a new Polyline composed of the given points, with the provided thickness.
     *
     * @param points    The points that will make up the Polyline.
     * @param thickness The thickness of the Polyline.
     */
    public Polyline(List<Vector> points, float thickness) {
        mPoints = new ArrayList<Vector>(points);
        float[][] array = new float[points.size()][3];
        for (int i = 0; i < points.size(); i++) {
            Vector p = points.get(i);
            array[i][0] = p.x;
            array[i][1] = p.y;
            array[i][2] = p.z;
        }

        mNativeRef = nativeCreatePolyline(array, thickness);
    }

    /**
     * @hide
     * @param points
     * @param width
     */
    //#IFDEF 'viro_react'
    public Polyline(float[][] points, float width) {
        mPoints = new ArrayList<Vector>();
        for (int i = 0; i < points.length; i++) {
            mPoints.add(new Vector(points[i][0], points[i][1], points[i][2]));
        }
        mNativeRef = nativeCreatePolyline(points, width);
    }
    //#ENDIF

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this Polyline.
     */
    public void dispose() {
        super.dispose();
    }

    /**
     * Set the points that compose this Polyline. This will reconstruct the
     * Polyline. To add one point at a time, it is faster to use {@link #appendPoint(Vector)}.
     *
     * @param points The points to set.
     */
    public void setPoints(List<Vector> points) {
        mPoints = points;

        float[][] array = new float[points.size()][3];
        for (int i = 0; i < points.size(); i++) {
            Vector p = points.get(i);
            array[i][0] = p.x;
            array[i][1] = p.y;
            array[i][2] = p.z;
        }
        nativeSetPoints(mNativeRef, array);
    }

    /**
     * Get the points in this Polyline.
     *
     * @return The points, each in a {@link Vector}.
     */
    public List<Vector> getPoints() {
        return mPoints;
    }

    /**
     * Set the thickness of this Polyline.
     *
     * @param thickness The thickness.
     */
    public void setThickness(float thickness) {
        mThickness = thickness;
        nativeSetThickness(mNativeRef, thickness);
    }

    /**
     * Get the thickness of this Polyline.
     *
     * @return The thickness.
     */
    public float getThickness() {
        return mThickness;
    }

    /**
     * Append the given point to this Polyline.
     *
     * @param point The point to append.
     */
    public void appendPoint(Vector point) {
        mPoints.add(point);
        nativeAppendPoint(mNativeRef, point.toArray());
    }

    private native long nativeCreatePolylineEmpty(float width);
    private native long nativeCreatePolyline(float[][] points, float width);
    private native void nativeAppendPoint(long lineReference, float[] point);
    private native void nativeSetPoints(long lineReference, float[][] points);
    private native void nativeSetThickness(long lineReference, float thickness);
}
