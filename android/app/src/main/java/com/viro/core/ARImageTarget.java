/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.graphics.Bitmap;

import java.util.HashMap;
import java.util.Map;

/**
 * ARImageTarget contains all the information required to find and track an image and
 * extract its pose information.
 */
public class ARImageTarget {

    public enum Orientation {
        Up("Up"),
        Down("Down"),
        Left("Left"),
        Right("Right");

        private String mStringValue;

        private Orientation(String value) {
            this.mStringValue = value;
        }

        public String getStringValue() {
            return mStringValue;
        }

        private static Map<String, ARImageTarget.Orientation> map = new HashMap<String, ARImageTarget.Orientation>();
        static {
            for (ARImageTarget.Orientation value : ARImageTarget.Orientation.values()) {
                map.put(value.getStringValue().toLowerCase(), value);
            }
        }
        /**
         * @hide
         */
        public static ARImageTarget.Orientation valueFromString(String str) {
            return map.get(str.toLowerCase());
        }
    }

    long mNativeRef;
    private final String mId;
    private final Orientation mOrientation;
    private final float mPhysicalWidth;

    /**
     * Creates a ARImageTarget object to be given to {@link ViroViewARCore} to start looking
     * for and tracking the specified image.
     *
     *
     * @param image the {@link Bitmap} containing the image to track.
     * @param orientation the orientation of given Bitmap.
     * @param physicalWidth the physical width of the image in meters (based on orientation).
     */
    public ARImageTarget(Bitmap image, Orientation orientation, float physicalWidth) {
        mId = String.valueOf(this.hashCode()); // the ID is simply the string of the hashCode().
        mOrientation = orientation;
        mPhysicalWidth = physicalWidth;
        mNativeRef = nativeCreateARImageTarget(image, mOrientation.getStringValue(), mPhysicalWidth, mId);
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
     * Release native resources associated with this ARImageTarget.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyARImageTarget(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Get the orientation of this ARImageTarget.
     */
    public Orientation getOrientation() {
        return mOrientation;
    }

    /**
     * Get the physicalWidth of this ARImageTarget.
     */
    public float getPhysicalWidth() {
        return mPhysicalWidth;
    }

    /**
     * Get the ID of this ARImageTarget which will be returned by {@link ARAnchor#getAnchorId()}
     * when this target is found and {@link ARScene.Listener#onAnchorFound(ARAnchor, ARNode)} is
     * invoked.
     */
    public String getId() {
        return mId;
    }

    private native long nativeCreateARImageTarget(Bitmap bitmap, String orientation,
                                                  float physicalWidth, String id);
    private native void nativeDestroyARImageTarget(long nativeRef);

}
