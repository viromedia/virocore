/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

public interface RenderCommandQueue {

    public void queueEvent(Runnable r);

}
