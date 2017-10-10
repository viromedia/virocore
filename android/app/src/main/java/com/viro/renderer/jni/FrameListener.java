package com.viro.renderer.jni;

/**
 * Copyright © 2017 Viro Media. All rights reserved.
 */

public abstract class FrameListener {
    protected long mNativeRef;

    public abstract void destroy();
}
