/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * ARImageAnchor is a specialized {@link ARAnchor} that represents a detected {@link ARImageTarget}
 * in the real world. To determine what {@link ARImageTarget} this anchor is tracking, compare
 * {@link ARAnchor#getAnchorId()} with each image target's {@link ARImageTarget#getId()}.
 */
public class ARImageAnchor extends ARAnchor {

    /**
     * Invoked from JNI
     * @hide
     */
    ARImageAnchor(String anchorId, String type, float[] position, float[] rotation, float[] scale) {
        super(anchorId, null, type, position, rotation, scale);
    }

}
