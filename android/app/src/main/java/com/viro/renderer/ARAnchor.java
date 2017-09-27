/*
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer;

/*
 A POJO containing all the data that an ARAnchor could contain.
 */
public class ARAnchor {
    // All ARAnchors have these props
    private float[] mPosition;
    private float[] mRotation; // in degrees
    private float[] mScale;

    // ARPlaneAnchors have these additional props
    private String mAlignment;
    private float[] mExtent;
    private float[] mCenter;

    public ARAnchor(float[] position, float[] rotation, float[] scale,
                    String alignment, float[] extent, float[] center) {
        mPosition = position;
        mRotation = rotation;
        mScale = scale;
        mAlignment = alignment;
        mExtent = extent;
        mCenter = center;
    }

    public float[] getPosition() {
        return mPosition;
    }

    public float[] getRotation() {
        return mRotation;
    }

    public float[] getScale() {
        return mScale;
    }

    public String getAlignment() {
        return mAlignment;
    }

    public float[] getExtent() {
        return mExtent;
    }

    public float[] getCenter() {
        return mCenter;
    }
}
