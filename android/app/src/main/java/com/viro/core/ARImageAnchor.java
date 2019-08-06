/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import java.util.HashMap;
import java.util.Map;

/**
 * ARImageAnchor is a specialized {@link ARAnchor} that represents a detected {@link ARImageTarget}
 * in the real world. To determine what {@link ARImageTarget} this anchor is tracking, compare
 * {@link ARAnchor#getAnchorId()} with each image target's {@link ARImageTarget#getId()}.
 */
public class ARImageAnchor extends ARAnchor {

    /**
     * Specifies the tracking method used to track the {@link ARImageAnchor}'s position.
     */
    public enum TrackingMethod {
        /**
         * The {@link ARImageAnchor} is not currently being tracked.
         */
        NOT_TRACKING("notTracking"),

        /**
         * The {@link ARImageAnchor} is currently being tracked actively.
         */
        TRACKING("tracking"),

        /**
         * The {@link ARImageAnchor} is not currently being tracked but its last known pose is being used.
         */
        LAST_KNOWN_POSE("lastKnownPose");

        private String mStringValue;

        private TrackingMethod(String value) {
            this.mStringValue = value;
        }

        /**
         * @hide
         */
        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, TrackingMethod> map = new HashMap<String, TrackingMethod>();
        static {
            for (TrackingMethod value : TrackingMethod.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }

        /**
         * @hide
         */
        public static TrackingMethod valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    };

    private final TrackingMethod mTrackingMethod;

    /**
     * Invoked from JNI
     * @hide
     */
    ARImageAnchor(String anchorId, String type, float[] position, float[] rotation, float[] scale, String trackingMethod) {
        super(anchorId, null, type, position, rotation, scale);
        mTrackingMethod = TrackingMethod.valueFromString(trackingMethod);
    }

    /**
     * Returns the current {@link TrackingMethod} used for this anchor.
     *
     * @return The tracking method used for this anchor.
     */
    public TrackingMethod getTrackingMethod() {
        return mTrackingMethod;
    }

}
