 /*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

 package com.viro.core.internal;

/**
 * Callback interface used to get camera related information from the
 * renderer asynchronously.
 */
public interface CameraCallback {
    void onGetCameraOrientation(float posX, float posY, float posZ,
                                float rotEulerX, float rotEulerY, float rotEulerZ,
                                float forwardX, float forwardY, float forwardZ,
                                float upX, float upY, float upZ);
}
