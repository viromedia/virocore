/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Interface for listening to frame events. The FrameListener can be registered to receive events
 * via {@link ViroView#setFrameListener(FrameListener)} in {@link ViroView}.
 */
public interface FrameListener {

    /**
     * Invoked whenever a frame is drawn.
     */
    public void onDrawFrame();

}
