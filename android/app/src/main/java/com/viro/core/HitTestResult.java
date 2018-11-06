package com.viro.core;


/**
 * HitTestResult encapsulates a single result of a virtual based hit-test. Virtual hit tests are initiated from
 * the {@link ViroView}. These hit tests are a mechanism to
 * discover what virtual world objects are contained along a given ray in the {@link
 * Scene} or at a given 2D point on the {@link
 * ViroView}.
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

    /**
      The point in world space where the source ray intersected a virtual object.
     **/
    public Vector getIntersectionPoint() {
        return mIntersectionPoint;
    }

    /**
      Distance from the start of the source ray.
     **/
    public float getDistance() {
        return mDistance;
    }

    /**
      The node tag {@link Node#getTag} that this hit test result intersected with. Used to identify
      the node intersected with.
     **/
    public String getTag() {
        return mNodeTag;
    }
}
