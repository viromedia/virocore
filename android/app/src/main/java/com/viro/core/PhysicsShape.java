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
 * The physics simulation approximates physics bodies with simplified shapes. These shapes are
 * approximations for performance reasons.
 */
public interface PhysicsShape {
    /**
     * @hide
     * @return
     */
    String getType();
    /**
     * @hide
     * @return
     */
    float[] getParams();
}