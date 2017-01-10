/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer;

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
