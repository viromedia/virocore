/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * HitTestResult encapsulates a single result of a virtual based hit-test. Virtual hit-tests are
 * hit-tests that intersect virtual (e.g. not real-world) objects, and are initiated from {@link
 * ViroView}. These hit tests are a mechanism to discover what virtual world objects are contained
 * along a given ray in the {@link Scene} or at a given 2D point on the {@link ViroView}.
 */
public class HitTestResult {

    private String mNodeTag;
    private Vector mIntersectionPoint;
    private float mDistance;

    /**
     * Invoked from Native.
     *
     * @param nodeTag
     * @param distance
     * @param intersectionPoint
     * @hide
     */
    public HitTestResult(String nodeTag, float distance, Vector intersectionPoint) {
        this.mNodeTag = nodeTag;
        this.mDistance = distance;
        this.mIntersectionPoint = intersectionPoint;
    }

    /**
     * Invoked from Native.
     *
     * @hide
     */
    HitTestResult(String nodeTag, float distance, float[] intersectionPoint) {
        mDistance = distance;
        mNodeTag = nodeTag;
        mIntersectionPoint = new Vector(intersectionPoint);
    }

    /**
     * Get the point in world space where the source ray intersected a virtual object.
     *
     * @return The intersection point of the hit-test, in world coordinates.
     */
    public Vector getIntersectionPoint() {
        return mIntersectionPoint;
    }

    /**
     * Get the distance from the origin of the intersection ray to the intersection point, in
     * world space.
     *
     * @return The distance between the origin of the intersection ray and the intersection point.
     */
    public float getDistance() {
        return mDistance;
    }

    /**
     * Get the tag of the Node that this hit-test intersected. Tags are used to identify the
     * intersected {@link Node}; they can be set via {@link Node#setTag(String)}.
     *
     * @return The tag of the intersected Node.
     */
    public String getTag() {
        return mNodeTag;
    }
}
