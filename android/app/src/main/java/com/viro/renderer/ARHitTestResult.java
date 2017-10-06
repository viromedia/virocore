/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer;

/*
 A POJO containing the data for an ARHitTestResult
 */
public class ARHitTestResult {

    private String mType;
    private float[] mPosition;
    private float[] mScale;
    private float[] mRotation;

    public ARHitTestResult(String type, float[] position, float[] scale, float[] rotation) {
        mType = type;
        mPosition = position;
        mScale = scale;
        mRotation = rotation;
    }

    public String getType() {
        return mType;
    }

    public float[] getPosition() {
        return mPosition;
    }

    public float[] getScale() {
        return mScale;
    }

    public float[] getRotation() {
        return mRotation;
    }

}
