/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
package com.viro.core;

/**
 * Callback interface for responding to Camera driven events. Created CameraListeners
 * can be registered via {@see com.viro.renderer#setCameraListener()}
 */
public interface CameraListener {

    /**
     * Invoked whenever the current Camera's transformations have changed.
     *
     * @param pos The last known position of the camera in world coordinates.
     * @param rot The last known rotation of the camera, represented as euler angles in Radians.
     * @param forward The last known forward direction of the camera.
     */
    void onTransformUpdate(Vector pos, Vector rot, Vector forward);
}