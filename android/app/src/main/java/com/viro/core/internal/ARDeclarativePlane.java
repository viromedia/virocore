/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

/**
 * @hide
 */
public class ARDeclarativePlane extends ARDeclarativeNode {

    public ARDeclarativePlane(float minWidth, float minHeight) {
        super();
        initWithNativeRef(nativeCreateARPlane(minWidth, minHeight));
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    @Override
    public void dispose() {
        super.dispose();
    }

    public void setMinWidth(float minWidth) {
        nativeSetMinWidth(mNativeRef, minWidth);
    }
    public void setMinHeight(float minHeight) {
        nativeSetMinHeight(mNativeRef, minHeight);
    }

    private native long nativeCreateARPlane(float minWidth, float minHeight);
    private native void nativeSetMinWidth(long nativeRef, float minWidth);
    private native void nativeSetMinHeight(long nativeRef, float minHeight);

}
