/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.renderer.jni;

import java.util.HashMap;
import java.util.Map;

/**
 * ARPlaneAnchor is a specialized {@link ARAnchor} that represents a detected plane in the
 * real-world. Viro will automatically construct a corresponding {@link ARDeclarativePlane} for each plane
 * anchor, and will automatically include that ARPlane in the scene. You can use the {@link ARDeclarativePlane}
 * as a real-world surface on which to place virtual content.
 */
public class ARPlaneAnchor extends ARAnchor {

    /**
     * Specifies the alignment of the ARPlaneAnchor with respect to gravity.
     */
    public enum Alignment {
        /**
         * The {@link ARPlaneAnchor} is horizontal, facing upward (e.g. a tabletop).
         */
        HORIZONTAL_UPWARD("HorizontalUpward"),

        /**
         * The {@link ARPlaneAnchor} is horizontal, facing downwrad (e.g. a ceiling).
         */
        HORIZONTAL_DOWNWARD("HorizontalDownward"),

        /**
         * The {@link ARPlaneAnchor} is not horizontal.
         */
        NON_HORIZONTAL("NonHorizontal");

        private String mStringValue;
        private Alignment(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, Alignment> map = new HashMap<String, Alignment>();
        static {
            for (Alignment value : Alignment.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         */
        public static Alignment valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    private Alignment mAlignment;
    private Vector mExtent;
    private Vector mCenter;

    /**
     * Invoked from JNI
     * @hide
     */
    public ARPlaneAnchor(String anchorId, String type, float[] position, float[] rotation,
                         float[] scale, String alignment, float[] extent, float[] center) {
        super(anchorId, type, position, rotation, scale);
        mAlignment = Alignment.valueFromString(alignment);
        mExtent = new Vector(extent);
        mCenter = new Vector(center);
    }

    /**
     * Get the {@link Alignment} of the plane with respect to gravity.
     *
     * @return The Alignment of the plane.
     */
    public Alignment getAlignment() {
        return mAlignment;
    }

    /**
     * Get the extent of the plane in the X, Y, and Z directions. This is the plane's estimated
     * size.
     *
     * @return The extent of the plane as a {@link Vector}.
     */
    public Vector getExtent() {
        return mExtent;
    }

    /**
     * Get the center of this plane, in world coordinates.
     *
     * @return The center as a {@link Vector}.
     */
    public Vector getCenter() {
        return mCenter;
    }
}
