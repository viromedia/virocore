/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Callback interface for responding to Camera driven events, such as position and rotation
 * changes. CameraListener can be registered to receive events via {@link ViroView#setCameraListener(CameraListener)}
 * in {@link ViroView}.
 */
public interface CameraListener {

    /**
     * Invoked whenever the current Camera's transformations have changed.
     *
     * @param position The last known position of the camera in world coordinates.
     * @param rotation The last known rotation of the camera, represented as Euler angles about each
     *                 principal axis, in radians.
     * @param forward  The last known forward direction of the camera. This unit vector indicates
     *                 the direction the camera is facing.
     */
    void onTransformUpdate(Vector position, Vector rotation, Vector forward);
}