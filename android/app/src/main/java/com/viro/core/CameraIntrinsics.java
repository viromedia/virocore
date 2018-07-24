package com.viro.core;


/**
 * Created by manish on 7/23/18.
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

    public float[] getFocalLength() {
        return mFocalLength;
    }

    public float[] getPrincipalPoint() {
        return mPrincipalPoint;
    }
}
