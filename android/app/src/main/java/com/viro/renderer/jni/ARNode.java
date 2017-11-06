/*
 * Copyright © 2017 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import java.lang.ref.WeakReference;

/**
 * ARNode is a specialized {@link Node} that corresponds to a detected {@link ARAnchor}. ARNodes are
 * automatically created by Viro and added to the {@link Scene} as they are detected, and by default
 * hold no content and have no children. Each ARNode is continually updated to stay in sync with its
 * corresponding ARAnchor: if the anchor's position, orientation, or other detected properties
 * change, the ARNode will be changed as well.
 * <p>
 * ARNode is the mechanism through which you can attach virtual content to real-world objects. For
 * example, if an {@link ARPlaneAnchor} is detected, you can add a 3D model to that plane by loading
 * the {@link Object3D} and making it a child of the ARNode.
 * <p>
 */
public abstract class ARNode extends Node {

    public ARNode() {
        super(false); // call the empty NodeJni constructor.
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
     * Release native resources associated with this ARNode.
     */
    @Override
    public void dispose() {
        super.dispose();
    }

    /**
     * Set to true to pause automatic synchronization between this ARNode and its {@link ARAnchor}.
     * ARAnchors are periodically updated by the AR tracking system as its estimates of the anchor's
     * properties are refined. By default, updates to the ARAnchor are synchronized to the ARNode;
     * for example, if the tracking system determines that an ARAnchor has moved, the ARNode will
     * move as well.
     * <p>
     * It may be useful to pause updates if you wish to ensure the stability of your {@link Scene}
     * for a period of time. The ARNode will be immediately updated to its anchor's latest position
     * when pause updates is turned off.
     *
     * @param pauseUpdates True to pause updates, false to resume updating. When set to false, the
     *                     ARNode will immediately be updated to match its {@link ARAnchor}.
     */
    public void setPauseUpdates(boolean pauseUpdates) {
        nativeSetPauseUpdates(mNativeRef, pauseUpdates);
    }

    private native void nativeSetPauseUpdates(long nativeRef, boolean pauseUpdates);

}
