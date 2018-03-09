/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import com.viro.core.internal.annotation.BridgeOnly;

import java.util.HashMap;
import java.util.Map;

/**
 * ARAnchor represents real-world objects whose position and orientation can be tracked. ARAnchors
 * are automatically created by Viro when AR features are detected. A corresponding {@link ARNode} is
 * created and added to the {@link Scene} for each detected anchor.
 */
public class ARAnchor {

    /**
     * Specifies the type of ARAnchor.
     */

    public enum Type {
        /**
         * Default type for an {@link ARAnchor}, representing a point and orientation.
         */
        ANCHOR("anchor"),

        /**
         * Plane type for an {@link ARAnchor}, representing a detected horizontal surface.
         */
        PLANE("plane"),

        /**
         * Image type for an {@link ARAnchor}, representing a detect {@link ARImageTarget}.
         */
        IMAGE("image");

        private String mStringValue;

        private Type(String value) {
            this.mStringValue = value;
        }

        /**
         * @hide
         */
        @BridgeOnly
        public String getStringValue() {
            return mStringValue;
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
        @BridgeOnly
        public static Type valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    // All ARAnchors have these props
    private String mAnchorId;

    private Type mType;

    private Vector mPosition; // world transform of the anchor

    private Vector mRotation; // in radians

    private Vector mScale;

    /**
     * Invoked from JNI
     * @hide
     */
    @BridgeOnly
    ARAnchor(String anchorId, String type, float[] position, float[] rotation, float[] scale) {
        mAnchorId = anchorId;
        mType = Type.valueFromString(type);
        mPosition = new Vector(position);
        mRotation = new Vector(rotation);
        mScale = new Vector(scale);
    }

    @Override
    protected void finalize() throws Throwable {
        try {

        } finally {
            super.finalize();
        }
    }

    /**
     * Get a unique String ID for this ARAnchor.
     *
     * @return Unique ID for the anchor.
     */
    public String getAnchorId() {
        return mAnchorId;
    }

    /**
     * Return the {@link Type} of this ARAnchor.
     *
     * @return The ARAnchor type.
     */
    public Type getType() {
        return mType;
    }

    /**
     * Get the position of this ARAnchor in world coordinates.
     *
     * @return The position of the ARAnchor as a {@link Vector}.
     */
    public Vector getPosition() {
        return mPosition;
    }

    /**
     * Get the rotation of this ARAnchor about each principal axis. The rotation defines the
     * anchor's orientation. The position, rotation, and scale of an ARAnchor define its
     * "pose", or transformation in the world.
     *
     * @return The three Euler rotation angles in a {@link Vector} in radians.
     */
    public Vector getRotation() {
        return mRotation;
    }

    /**
     * Get the scale of this ARAnchor in the X, Y, and Z dimensions. The position, rotation, and
     * scale of an ARAnchor define its "pose", or transformation in the world.
     *
     * @return The scale of this anchor.
     */
    public Vector getScale() {
        return mScale;
    }

}
