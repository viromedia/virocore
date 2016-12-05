/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public abstract class BaseGeometry {
    protected long mNativeRef;
    public abstract void attachToNode(NodeJni jni);
}