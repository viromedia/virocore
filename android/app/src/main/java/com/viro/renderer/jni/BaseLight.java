/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

public abstract class BaseLight {
    protected long mNativeRef;
    public abstract void addToNode(NodeJni jni);
    public abstract void removeFromNode(NodeJni jni);
}
