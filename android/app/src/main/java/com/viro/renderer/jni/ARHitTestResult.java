/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
package com.viro.renderer.jni;

import com.viro.renderer.jni.Texture;
import com.viro.renderer.jni.Vector;

import java.util.HashMap;
import java.util.Map;

/**
 * ARHitTestResult encapsulates a single result of an AR hit-test. AR hit-tests are initiated from
 * the {@link com.viro.renderer.jni.ViroViewARCore}. These hit tests are a mechanism for you to
 * discover what real-world objects are contained along a given ray in the {@link
 * com.viro.renderer.jni.Scene} or at a given 2D point on the {@link
 * com.viro.renderer.jni.ViroViewARCore}.
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

    private Type mType;
    private Vector mPosition;
    private Vector mScale;
    private Vector mRotation;

    /**
     * Invoked by Native.
     * @hide
     */
    ARHitTestResult(String type, float[] position, float[] scale, float[] rotation) {
        mType = Type.valueFromString(type);
        mPosition = new Vector(position);
        mScale = new Vector(scale);
        mRotation = new Vector(rotation);
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

}
