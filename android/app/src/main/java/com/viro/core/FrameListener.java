/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Callback interface for listening to frame events. FrameListener can be used to receive a callback
 * just before each frame is rendered. FrameListener is registered via {@link
 * ViroView#setFrameListener(FrameListener)} in {@link ViroView}.
 */
public interface FrameListener {

    /**
     * Invoked just before a frame is drawn.
     */
    public void onDrawFrame();

}
