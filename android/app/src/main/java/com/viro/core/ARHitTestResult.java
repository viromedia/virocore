/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.util.Log;

import java.util.HashMap;
import java.util.Map;

/**
 * ARHitTestResult encapsulates a single result of an AR hit-test. AR hit-tests are initiated from
 * the {@link ViroViewARCore}. These hit tests are a mechanism for you to
 * discover what real-world objects are contained along a given ray in the {@link
 * Scene} or at a given 2D point on the {@link
 * ViroViewARCore}.
 */
public class ARHitTestResult {

    /**
     * Hit-tests can intersect different kinds of real-world features, each identified by their
     * Type.
     */
    public enum Type {
        /**
         * Indicates a feature-point was found by the hit-test. Feature points are points that have
         * been identified as part of a continuous surface, but have not yet been correlated into
         * a plane.
         */
        FEATURE_POINT("FeaturePoint"),
        /**
         * Indicates a plane was found by the hit-test.
         */
        PLANE("ExistingPlaneUsingExtent");

        private final String mStringValue;

        public static Type forString(String string) {
            for (Type format : Type.values()) {
                if (format.getStringValue().equalsIgnoreCase(string)) {
                    return format;
                }
            }
            throw new IllegalArgumentException("Invalid ARHitTestResult.Type [" + string + "]");
        }

        private Type(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         */
        public String getStringValue() {
            return this.mStringValue;
        }

        private static Map<String, Type> map = new HashMap<String, Type>();
        static {
            for (Type value : Type.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         */
        public static Type valueFromString(String str) {
            return str == null ? null : map.get(str.toLowerCase());
        }
    }

    private long mNativeRef;
    private Type mType;
    private Vector mPosition;
    private Vector mScale;
    private Vector mRotation;
    private ARNode mARNode;

    /**
     * Invoked by Native.
     * @hide
     */
    ARHitTestResult(long nativeRef, String type, float[] position, float[] scale, float[] rotation) {
        mNativeRef = nativeRef;
        mType = Type.valueFromString(type);
        mPosition = new Vector(position);
        mScale = new Vector(scale);
        mRotation = new Vector(rotation);
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
     * Release native resources associated with this ARHitTestResult.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyARHitTestResult(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Returns the kind of detected real-world object contained in this result.
     *
     * @return The type of the real-world object.
     */
    public Type getType() {
        return mType;
    }

    /**
     * Returns the position of the detected real-world object, in world coordinates.
     *
     * @return The position in world coordinates, in a {@link Vector}.
     */
    public Vector getPosition() {
        return mPosition;
    }

    /**
     * Return the scale of the detected real-world object.
     *
     * @return The scale in a {@link Vector}.
     */
    public Vector getScale() {
        return mScale;
    }

    /**
     * Return the orientation of the detected real-world object, along each of the three
     * principal axes.
     *
     * @return The rotation in radians along the X, Y, and Z axes, stored in a {@link Vector}.
     */
    public Vector getRotation() {
        return mRotation;
    }

    /**
     * Create an {@link ARNode} that will be anchored to the real-world position associated with
     * this hit result. Anchoring a node to a hit-result will help ensure that the objects you
     * attach to this node will track properly and remain in place.
     * <p>
     * Note that the returned {@link ARNode} is automatically added to the Scene, and will be
     * continually updated to stay in the sync with its underlying anchor as the anchor's
     * properties, orientation, or position change.
     * <p>
     * If there is already an ARNode associated with this hit result, that ARNode will be returned
     * here.
     * <p>
     * When finished with this ARNode, you must call {@link ARNode#detach()} to remove it from
     * the system. If you do not detach the ARNode, it will continue to receive tracking updates
     * from the AR subsystem, adversely impacting performance.
     * <p>
     * @return New {@link ARNode} anchored to the hit result position.
     */
    public ARNode createAnchoredNode() {
        if (mARNode == null) {
            mARNode = new ARNode(nativeCreateAnchoredNode(mNativeRef));
        }
        return mARNode;
    }

    private native long nativeCreateAnchoredNode(long hitResultRef);
    private native void nativeDestroyARHitTestResult(long hitResultRef);

}
