//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.core;

import android.graphics.Point;
import android.util.Log;

import java.util.concurrent.ConcurrentHashMap;

/**
 * ARNode is a specialized {@link Node} that corresponds to a detected or manually created {@link
 * ARAnchor}. ARNodes are automatically created by Viro and added to the {@link Scene} as anchors
 * are created, and by default hold no content and have no children. Each ARNode is continually
 * updated to stay in sync with its corresponding ARAnchor: if the anchor's position, orientation,
 * or other detected properties change, the ARNode will be changed as well.
 * <p>
 * ARNode is the mechanism through which you can attach virtual content to real-world objects. For
 * example, if an {@link ARPlaneAnchor} is detected, you can add a 3D model to that plane by loading
 * the {@link Object3D} and making it a child of the anchor's ARNode.
 * <p>
 * While normal {@link Node}s can be used in AR, it is highly recommended to ensure that all nodes
 * descend from an ARNode. This is because the world coordinate system used by the underlying
 * tracking technology may not be stable. By using an {@link ARNode}, you ensure that said node is
 * continually tracked, maintaining its position in the real world. Normal {@link Node} objects can
 * be added as children to the ARNode, and they too will maintain their position relative to the
 * real-world.
 * <p>
 * There are three ways to acquire ARNodes: <ol> <li>Real-world feature detection. Every real-world
 * feature detected will trigger a callback giving you an ARNode corresponding to that feature. This
 * can used to add content to real-world features like planes or images. To do this, attach a {@link
 * ARScene.Listener} to the {@link ARScene}, and listen for {@link ARScene.Listener#onAnchorFound(ARAnchor,
 * ARNode)}. </li> <li>Hit-test results. Perform a hit-test using {@link
 * ViroViewARCore#performARHitTest(Point, ARHitTestListener)}, {@link
 * ViroViewARCore#performARHitTestWithPosition(Vector, ARHitTestListener)}, or {@link
 * ViroViewARCore#performARHitTestWithRay(Vector, ARHitTestListener)}. These functions will return
 * an {@link ARHitTestResult} for each intersected real-world. You can retrieve an ARNode
 * corresponding to any hit location via {@link ARHitTestResult#createAnchoredNode()}.</li>
 * <li>Arbitrary real-world location. You can create an ARNode at any world coordinate position by
 * invoking {@link ARScene#createAnchoredNode(Vector)}. Note that world coordinates are in
 * meters.</li>
 * <p>
 * <p>
 * Finally, when finished with a manually constructed ARNode (methods 2 or 3 above), you must call
 * {@link ARNode#detach()} to remove it from the system. If you do not detach the ARNode, it will
 * continue to receive tracking updates from the AR subsystem, adversely impacting performance.
 * Nodes that are automatically detected (method 1 above), do not need to be detached.
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
     * from receiving further AR tracking updates. After this is called, the ARNode becomes
     * unusable.
     * <p>
     * This may only be called on ARNodes that are created manually by calls to {@link
     * ARHitTestResult#createAnchoredNode()}, {@link ARScene#createAnchoredNode(Vector,
     * Quaternion)}, or {@link ARScene#createAnchoredNode(Vector)}. Other ARNodes are created
     * automatically when Viro detects real-world features (e.g. planes and images), and are
     * automatically detached when those features are no longer visible to the AR system.
     */
    public void detach() {
        if (!nativeIsAnchorManaged(mNativeRef)) {
            nativeDetach(mNativeRef);
        } else {
            Log.w("Viro", "Ignoring invalid request to detach a managed ARNode");
        }
    }

    @Override
    public void removeFromParentNode() {
        // ARNodes cannot be manually removed
        // TODO Apparently ViroReact does this all the time. Figure it out.
        //throw new IllegalAccessError("Viro: Invalid attempt to remove an ARNode from the Scene");
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
