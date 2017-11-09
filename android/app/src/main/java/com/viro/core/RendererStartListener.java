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
 * Callback interface for responding to the Viro renderer finishing initialization. This listener
 * are used by the subclasses of {@link ViroView}.
 */
public interface RendererStartListener {

    /**
     * Callback invoked when {@link ViroView} has finished initialization. When this is received,
     * the ViroView is ready to begin rendering content.
     */
    void onRendererStart();
}
