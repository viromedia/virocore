/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * @hide
 */
abstract class NativeFrameListener {
    protected long mNativeRef;
    public abstract void destroy();
}
