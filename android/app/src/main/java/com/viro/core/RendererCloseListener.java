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
 * Callback runnable that is used for responding to Viro renderer being closed by the user.
 * This listener is used by {@link ViroViewGVR}
 */
public interface RendererCloseListener {

    void onRendererClosed();
}
