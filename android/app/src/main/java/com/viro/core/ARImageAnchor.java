//
//  Copyright (c) 2018-present, ViroMedia, Inc.
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
