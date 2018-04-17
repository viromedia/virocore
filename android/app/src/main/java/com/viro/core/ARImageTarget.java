/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.graphics.Bitmap;

import java.util.HashMap;
import java.util.Map;

/**
 * ARImageTarget contains all the information required to find and track an image and extract its
 * pose information. To use image targets:
 * <p>
 *
 * <ol>
 * </ul><p>
 * <li>Create an ARImageTarget representing the image you would like to find in the real-world.
 * <li>Add the ARImageTarget to your app by using {@link ARScene#addARImageTarget(ARImageTarget)}.
 * <li>Once an image target is added to the scene, you will receive a callback when an instance of that
 * image is detected in the world. The callback will come through {@link
 * com.viro.core.ARScene.Listener#onAnchorFound(ARAnchor, ARNode)}, with an anchor of type {@link
 * ARImageAnchor}.
 * <li>Use {@link ARImageAnchor#getAnchorId()} and compare against {@link
 * ARImageTarget#getId()} to identify which of your targets was found in the world.</ol><p>
 *
 * Note that for each {@link ARImageTarget} added to the scene, there will be only one instance of
 * that target found.
 */
public class ARImageTarget {

    /**
     * The orientation of the image to detect in the real world, with respect to the base orientation
     * of your input image. For example, if the input image is an upside-down dollar bill, you would set the
     * orientation to {@link #Down} to identify a right-side-up dollar bill. The various orientations
     * refer to where the <i>top</i> of the image is.
     */
    public enum Orientation {
        /**
         * The top of the image is at the top. Represents no rotation of the image.
         */
        Up("Up"),
        /**
         * The top of the image is at the bottom. Represents 180 degree rotation of the image.
         */
        Down("Down"),
        /**
         * The top of the image is on the left. Represents 90 degree rotation of the image
         * counterclockwise.
         */
        Left("Left"),
        /**
         * The top of the image is on the right. Represents 90 degree rotation of the image
         * clockwise.
         */
        Right("Right");

        private String mStringValue;
        private Orientation(String value) {
            this.mStringValue = value;
        }

        /**
         * @hide
         * @return
         */
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

    private long mNativeRef;
    private final String mId;
    private final Orientation mOrientation;
    private final float mPhysicalWidth;

    /**
     * @hide
     */
    public long getNativeRef() {
        return mNativeRef;
    }

    /**
     * Creates a ARImageTarget object to be given to {@link ViroViewARCore} to start looking
     * for and tracking the specified image.
     *
     * @param image         The {@link Bitmap} containing the image to track.
     * @param orientation   The {@link Orientation} of the given Bitmap, which indicates where the
     *                      top of the image is. For example, if the input image is an upside-down
     *                      dollar bill, you would set the orientation to {@link Orientation#Down}
     *                      to identify right-side-up dollar bills in the world. Set to {@link
     *                      Orientation#Up} to indicate you want to identify the image as-is without
     *                      any rotation.
     * @param physicalWidth The real-world width of the image in meters. Note this is the width
     *                      <i>post</i>-orientation.
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
     * Get the {@link Orientation} of this ARImageTarget.
     *
     * @return The orientation of this target.
     */
    public Orientation getOrientation() {
        return mOrientation;
    }

    /**
     * Get the physical width (in meters) of this ARImageTarget. This width is
     * <i>post</i>-orientation.
     *
     * @return The physical width in meters.
     */
    public float getPhysicalWidth() {
        return mPhysicalWidth;
    }

    /**
     * Get the ID of this ARImageTarget, which corresponds to the ID returned by {@link
     * ARAnchor#getAnchorId()} when this target is found.
     *
     * @return The ID of this target.
     */
    public String getId() {
        return mId;
    }

    private native long nativeCreateARImageTarget(Bitmap bitmap, String orientation, float physicalWidth,
                                                  String id);
    private native void nativeDestroyARImageTarget(long nativeRef);

}
