/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import android.util.Log;

import java.util.concurrent.ConcurrentHashMap;

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
 * To get an ARNode, attach a {@link ARScene.Listener} to the {@link ARScene},
 * and listen for {@link ARScene.Listener#onAnchorFound(ARAnchor, ARNode)},
 * which is invoked each time a new {@link ARAnchor} is found, with its corresponding {@link
 * ARNode}.
 */
public class ARNode extends Node {

    /**
     * ARNodes need strong references to native created ARNodes since their Java representation
     * typically isn't stored anywhere. We remove ARNodes from this map as they are removed by the
     * AR tracking engine.
     *
     * @hide
     */
    protected static ConcurrentHashMap<Integer, ARNode> nodeARMap = new ConcurrentHashMap<Integer, ARNode>();

    /**
     * @hide
     */
    static ARNode getARNodeWithID(int id) {
        return nodeARMap.get(id);
    }

    /**
     * @hide
     */
    static ARNode removeARNodeWithID(int id) {
        return nodeARMap.remove(id);
    }

    /**
     * Protected constructor for subclasses (declarative nodes).
     * @hide
     */
    protected ARNode() {
        super(false);
    }

    /**
     * Protected constructor to wrap an ARNode created natively.
     * @hide
     * @param nativeRef
     */
    ARNode(long nativeRef) {
        super(false); // call the empty Node constructor.

        initWithNativeRef(nativeRef);
        nodeARMap.put(nativeGetUniqueIdentifier(mNativeRef), this);
    }

    @Override
    protected void finalize() throws Throwable {
        try {

        } finally {
            super.finalize();
        }
    }

    /**
     * Detaches this ARNode, removing it and all of its children from the Scene, and stopping it
     * from receiving further AR tracking updates. After this is called, the ARNode becomes unusable.
     * This may only be called on ARNodes that are created manually by calls to
     * {@link ARHitTestResult#createAnchoredNode()}. Other ARNodes are created automatically when
     * Viro detects real-world features, and are automatically detached when those features are no
     * longer visible to the AR system.
     */
    public void detach() {
        if (!nativeIsAnchorManaged(mNativeRef)) {
            removeARNodeWithID(nativeGetUniqueIdentifier(mNativeRef));
            nativeDetach(mNativeRef);
        } else {
            Log.w("Viro", "Ignoring invalid request to detach a managed ARNode");
        }
    }

    @Override
    public void removeFromParentNode() {
        // ARNodes cannot be manually removed
        throw new IllegalAccessError("Viro: Invalid attempt to remove an ARNode from the Scene");
    }

    /**
     * Get the {@link ARAnchor} associated with this ARNode. The anchor is used to fix the
     * virtual content of this Node to tracked object represented by the anchor. This node is
     * automatically updated with the anchor's transformations.
     * <p>
     * This returns an immutable copy of the underlying anchor. The anchor returned will not
     * be updated by the tracking system: it is snapshot of the anchor at this point in time.
     * To get the latest transforms, you must invoke getAnchor() again.
     * <p>
     *
     * @return The {@link ARAnchor} to which this ARNode is attached.
     */
    public ARAnchor getAnchor() {
        return nativeGetAnchor(mNativeRef);
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

    /**
     * This operation is not valid for ARNodes. This is because their transforms are constantly
     * updated by the underlying AR tracking system. Calling this function will throw an IllegalAccessError.
     */
    @Override
    public void setPosition(Vector position) {
        throw new IllegalAccessError("Viro: Invalid attempt to set a position for an ARNode!");
    }

    /**
     * This operation is not valid for ARNodes. This is because their transforms are constantly
     * updated by the underlying AR tracking system. Calling this function will throw an IllegalAccessError.
     */
    @Override
    public void setRotation(Vector rotation) {
        throw new IllegalAccessError("Viro: Invalid attempt to set a rotation for an ARNode!");
    }

    private native ARAnchor nativeGetAnchor(long nativeRef);
    private native void nativeDetach(long nativeRef);
    private native void nativeSetPauseUpdates(long nativeRef, boolean pauseUpdates);
    private native boolean nativeIsAnchorManaged(long nativeRef);

}
