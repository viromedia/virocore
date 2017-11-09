/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
package com.viro.core;

public interface AsyncObject3DListener {

    /**
     * Invoked on main thread after the {@link Object3D} is loaded.
     *
     * @param object The Object3D that was loaded.
     * @param type   The type of Object3D.
     */
    void onObject3DLoaded(Object3D object, Object3D.Type type);

    /**
     * Invoked on the main thread if an {@link Object3D} fails to load.
     *
     * @param error The error message.
     */
    void onObject3DFailed(String error);

}
