//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.core;

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
         * Image type for an {@link ARAnchor}, representing a detected {@link ARImageTarget}.
         */
        IMAGE("image");

        private String mStringValue;

        private Type(String value) {
            this.mStringValue = value;
        }

        /**
         * @hide
         */
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

    // Cloud anchor properties
    private String mCloudAnchorId;

    /**
     * Invoked from JNI
     * @hide
     */
    ARAnchor(String anchorId, String cloudAnchorId, String type,
             float[] position, float[] rotation, float[] scale) {
        mAnchorId = anchorId;
        mType = Type.valueFromString(type);
        mPosition = new Vector(position);
        mRotation = new Vector(rotation);
        mScale = new Vector(scale);
        mCloudAnchorId = cloudAnchorId;
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
     * Return the cloud identifier of this anchor. This identifier is only present if this anchor
     * is hosted on the cloud or was resolved from the cloud. If this anchor is not a cloud anchor,
     * this method will return null.
     * <p>
     * You use the cloud anchor ID to resolve anchors that other devices have hosted. See
     * {@link ARScene#resolveCloudAnchor(String, ARScene.CloudAnchorResolveListener)}.
     * <p>
     * @return The cloud anchor ID of this anchor, or null if this anchor is not in the cloud.
     */
    public String getCloudAnchorId() { return mCloudAnchorId; }

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
