/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */
package com.viro.core.internal;

import android.opengl.GLSurfaceView;

public class GLSurfaceViewQueue implements RenderCommandQueue {

    private GLSurfaceView mSurfaceView;

    public GLSurfaceViewQueue(GLSurfaceView surfaceView) {
        this.mSurfaceView = surfaceView;
    }

    @Override
    public void queueEvent(Runnable r) {
        mSurfaceView.queueEvent(r);
    }
}
