/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
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
