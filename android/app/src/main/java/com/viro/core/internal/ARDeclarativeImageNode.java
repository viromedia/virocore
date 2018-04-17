/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import com.viro.core.ARImageTarget;

/**
 * @hide
 *
 * This class represents an ARDeclarativeNode that should be matched with
 * the given ARImageTarget
 */
public class ARDeclarativeImageNode extends ARDeclarativeNode {

    private ARImageTarget mARImageTarget;

    public ARDeclarativeImageNode() {
        initWithNativeRef(nativeCreateARImageTargetNode());
    }

    public void setARImageTarget(ARImageTarget arImageTarget) {
        mARImageTarget = arImageTarget;
        nativeSetARImageTarget(mNativeRef, arImageTarget.getNativeRef());
    }

    public ARImageTarget getARImageTarget() {
        return mARImageTarget;
    }

    private native long nativeCreateARImageTargetNode();
    private native void nativeSetARImageTarget(long nativeRef, long arImageTargetRef);
}
