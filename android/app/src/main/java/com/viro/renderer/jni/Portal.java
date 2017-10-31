/*
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

/**
 * Portal is a special {@link Node} that serves as an entry or window into an associated
 * {@link PortalScene}.
 * <p>
 * The Portal's contents (its subnodes and geometry) should be a 3D representation of a
 * window or door, aligned with the X axis in its local coordinate system. The transparent
 * sections of the Portal will display the {@link PortalScene} for which the Portal serves
 * as an entrance.
 *
 * @see PortalScene
 */
public class Portal extends Node {

    /**
     * Construct a new Portal.
     */
    public Portal() {
        super(false);
        setNativeRef(nativeCreatePortal());
    }

    @Override
    protected void finalize() throws Throwable {
        try {
            dispose();
        } finally {
            super.finalize();
        }
    }

    /**
     * Release native resources associated with this Portal.
     */
    public void dispose() {
        if (mNativeRef != 0) {
            nativeDestroyPortal(mNativeRef);
            mNativeRef = 0;
        }
    }

    private native long nativeCreatePortal();
    private native void nativeDestroyPortal(long portalRef);
}
