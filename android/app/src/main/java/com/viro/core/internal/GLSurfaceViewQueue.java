/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import android.opengl.GLSurfaceView;

public class GLSurfaceViewQueue implements RenderCommandQueue {
    private GLSurfaceView mSurfaceView = null;
    private GLTextureView mTextureView = null;

    public GLSurfaceViewQueue(GLSurfaceView surfaceView) {
        mSurfaceView = surfaceView;
    }

    public GLSurfaceViewQueue(GLTextureView surfaceView) {
        mTextureView = surfaceView;
    }

    @Override
    public void queueEvent(Runnable r) {
        if (mSurfaceView != null){
            mSurfaceView.queueEvent(r);
        } else if (mTextureView != null){
            mTextureView.queueEvent(r);
        }
    }
}
