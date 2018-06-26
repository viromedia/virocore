/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * ARPlaneAnchor is a specialized {@link ARAnchor} that represents a detected plane in the
 * real-world. You can use the {@link ARNode} corresponding to an ARPlane as a real-world surface
 * on which to place virtual content.
 */
public class ARPlaneAnchor extends ARAnchor {

    /**
     * Specifies the alignment of the ARPlaneAnchor with respect to gravity.
     */
    public enum Alignment {
        /**
         * The {@link ARPlaneAnchor} is horizontal.
         */
        HORIZONTAL("Horizontal"),

        /**
         * The {@link ARPlaneAnchor} is horizontal, facing upward (e.g. a tabletop).
         */
        HORIZONTAL_UPWARD("HorizontalUpward"),

        /**
         * The {@link ARPlaneAnchor} is horizontal, facing downward (e.g. a ceiling).
         */
        HORIZONTAL_DOWNWARD("HorizontalDownward"),

        /**
         * The {@link ARPlaneAnchor} is not horizontal.
         */
        VERTICAL("Vertical");

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
    private ArrayList<Vector> mVertices;

    /**
     * Invoked from JNI
     * @hide
     */
    ARPlaneAnchor(String anchorId,
                  String type,
                  float[] position,
                  float[] rotation,
                  float[] scale,
                  String alignment,
                  float[] extent,
                  float[] center,
                  float[] boundaryVerticesArray) {
        super(anchorId, null, type, position, rotation, scale);
        mAlignment = Alignment.valueFromString(alignment);
        mExtent = new Vector(extent);
        mCenter = new Vector(center);
        mVertices = new ArrayList<Vector>();

        for (int i = 0; i < boundaryVerticesArray.length / 3 ; i ++) {
            Vector point = new Vector();
            point.x = boundaryVerticesArray[i*3 + 0];
            point.y = boundaryVerticesArray[i*3 + 1];
            point.z = boundaryVerticesArray[i*3 + 2];
            mVertices.add(point);
        }
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

    /**
     * Get the vertices composing the boundary of this polygonal plane.
     *
     * @return The boundary vertices in a {@link List}, which each vertex represented as a {@link
     * Vector}.
     */
    public List<Vector> getVertices() {
        return mVertices;
    }
}
