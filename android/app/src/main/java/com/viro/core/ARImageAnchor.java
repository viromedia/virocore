/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * ARImageAnchor is an {@link ARAnchor} that represents a detect {@link ARImageTarget} in the real
 * world. You can use {@link ARAnchor#getAnchorId()} to compare with {@link ARImageTarget#getId()}
 * to determine which {@link ARImageTarget} this ARAnchor is tracking.
 */
public class ARImageAnchor extends ARAnchor {

    /**
     * Invoked from JNI
     * @hide
     */
    ARImageAnchor(String anchorId, String type, float[] position, float[] rotation, float[] scale) {
        super(anchorId, type, position, rotation, scale);
    }

}
