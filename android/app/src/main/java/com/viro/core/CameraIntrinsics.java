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
