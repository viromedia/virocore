package com.viro.core;

/**
 *
 * Created by vadvani on 10/10/18.
 */

public class HitTestResult {

    private String mNodeTag;
    private Vector mIntersectionPoint;
    private float mDistance;

    public HitTestResult(String nodeTag, float distance, Vector intersectionPoint) {
        this.mNodeTag = nodeTag;
        this.mDistance = distance;
        this.mIntersectionPoint = intersectionPoint;
    }

    /**
     * Invoked by Native.
     * @hide
     */
    HitTestResult(String nodeTag, float distance, float[] intersectionPoint) {

        mDistance = distance;
        mNodeTag = nodeTag;
        mIntersectionPoint = new Vector(intersectionPoint);
    }

    public Vector getIntersectionPoint() {
        return mIntersectionPoint;
    }

    public float getDistance() {
        return mDistance;
    }

    public String getTag() {
        return mNodeTag;
    }
}
