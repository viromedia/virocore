/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core.internal;

import com.viro.core.ARPlaneAnchor;

/**
 * @hide
 */
//#IFDEF 'viro_react'
public class ARDeclarativePlane extends ARDeclarativeNode {

    public ARDeclarativePlane(float minWidth, float minHeight, ARPlaneAnchor.Alignment alignment) {
        super();
        initWithNativeRef(nativeCreateARPlane(minWidth, minHeight, alignment.getStringValue()));
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        }
        finally {
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
    public void setAlignment(ARPlaneAnchor.Alignment alignment) {
        nativeSetAlignment(mNativeRef, alignment.getStringValue());
    }

    private native long nativeCreateARPlane(float minWidth, float minHeight, String alignment);
    private native void nativeSetMinWidth(long nativeRef, float minWidth);
    private native void nativeSetMinHeight(long nativeRef, float minHeight);
    private native void nativeSetAlignment(long nativeRef, String alignment);

}
//#ENDIF