/*
 * Copyright © 2016 Viro Media. All rights reserved.
 */
/*
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.NodeJni.java
 * Cpp JNI wrapper      : Node_JNI.cpp
 * Cpp Object           : VRONode.cpp
 */
package com.viro.renderer.jni;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Underlying each {@link Scene} is a full-featured 3D scene graph engine. A scene graph
 * is a hierarchical tree structure of nodes that allows developers to intuitively construct a 3D
 * environment. The root node is retrieved via {@link Scene#getRootNode()}. Sub-nodes are
 * represented by child Node objects. Each Node represents a position and transform in 3D space, to
 * which you can attach 3D objects, lights, or other content.
 * <p>
 * A Node by itself has no visible content when it is rendered; it represents only a coordinate
 * space transform (position, rotation, and scale) relative to its parent Node. You use a hierarchy
 * of Node objects to model your scene in a way that makes sense for your application.
 * <p>
 * To take a concrete example, suppose your app presents an animated view of a solar system.
 * You can construct a Node hierarchy that models the sun, planets, and moons relative to one another.
 * Each body can be a Node, with its position in its orbit defined in the coordinate system of
 * its parent. The sun would define its own coordinate space, and the Earth would position itself in
 * that space. At the same time, the Earth would define its own coordinate space in which the moon
 * would position itself, and so on.
 */
public class Node {

    /**
     * Specifies the behavior of dragging if dragging is enabled on a Node's {@link EventDelegate}.
     */
    public enum DragType {
        /**
         * Dragging is limited to a fixed radius around the user. That is, as you drag a Node
         * around, it never changes its distance from the user: it stays fixed as though you are
         * dragging the Node around the inner surface of sphere.
         */
        FIXED_DISTANCE("FixedDistance"),

        /**
         * Dragging is based on intersection with real-world object. This is only available in
         * AR. This setting is used when ou wish to drag a Node off a table and onto the ground,
         * for example.
         */
        FIXED_TO_WORLD("FixedToWorld");

        private String mStringValue;
        private DragType(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }
    };

    /**
     * TransformBehaviors change the way a Node computes its position, rotation and scale. Multiple
     * TransformBehaviors can be attached to a Node at once.
     */
    public enum TransformBehavior {
        /**
         * Swivels the Node about both the X and Y axes to face the user. When this is set, the Node
         * will ignore its X and Y axis rotation.
         */
        BILLBOARD("billboard"),

        /**
         * Swivels the Node about its X axis to face the user. The Node's Y axis rotation will
         * remain unchanged.
         */
        BILLBOARD_X("billboardX"),

        /**
         * Swivels the Node about its Y axis to face the user. The Node's X axis rotation will
         * remain unchanged.
         */
        BILLBOARD_Y("billboardY");

        private String mStringValue;
        private TransformBehavior(String value) {
            this.mStringValue = value;
        }
        /**
         * @hide
         * @return
         */
        public String getStringValue() {
            return mStringValue;
        }
    };

    private static long INVALID_REF = Long.MAX_VALUE;
    protected long mNativeRef;

    protected boolean mDestroyed = false;
    private EventDelegate mEventDelegate = null;
    private Geometry mGeometry;
    private PhysicsBody mPhysicsBody;
    private WeakReference<Node> mParent;
    private ArrayList<Node> mChildren = new ArrayList<Node>();
    private Vector mScalePivot;
    private Vector mRotationPivot;
    private float mOpacity = 1.0f;
    private boolean mVisible = true;
    private DragType mDragType = DragType.FIXED_DISTANCE;
    private int mLightReceivingBitMask = 1;
    private int mShadowCastingBitMask = 1;
    private boolean mHighAccuracyGaze = false;
    private boolean mIgnoreEventHandling = false;
    private EnumSet<TransformBehavior> mTransformBehaviors;
    private String mTag;
    private Camera mCamera;

    /**
     * Construct a new Node centered at the origin, with no geometry.
     */
    public Node() {
        mNativeRef = nativeCreateNode();
    }

    /**
     * @hide
     * This constructor to be called by child classes that want to
     * override mNativeRef
     */
    protected Node(boolean dummyArg) {
        // no-op
    }

    /**
     * @hide Function called by child classes to set mNativeRef
     */
    protected void setNativeRef(long nativeRef) {
        mNativeRef = nativeRef;
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
     * Release native resources associated with this Node.
     */
    public void dispose() {
        mDestroyed = true;
        removeTransformDelegate();

        if (mNativeRef != 0) {
            nativeDestroyNode(mNativeRef);
            mNativeRef = 0;
        }
    }

    /**
     * Set the {@link EventDelegate} to use for this Node. EventDelegate objects respond to
     * input events like cliking, dragging, and moving.
     *
     * @param eventDelegate The {@link EventDelegate} to use for input events received on this Node.
     */
    public void setEventDelegate(EventDelegate eventDelegate) {
        if (mEventDelegate != null) {
            mEventDelegate.destroy();
        }

        mEventDelegate = eventDelegate;
        if (mEventDelegate != null) {
            nativeSetEventDelegate(mNativeRef, mEventDelegate.mNativeRef);
        }
    }

    /**
     * Add a child Node to this Node. The child will exist in the coordinate space of this Node
     * (that is, it inherits the rotation, scale, and position of its parent, and applies its own
     * rotation, scale, and position).
     *
     * @param childNode The Node to add as a child.
     */
    public void addChildNode(Node childNode) {
        mChildren.add(childNode);
        childNode.mParent = new WeakReference<Node>(this);
        nativeAddChildNode(mNativeRef, childNode.mNativeRef);
    }

    /**
     * Remove this Node from its parent.
     */
    public void removeFromParentNode() {
        Node parent = mParent != null ? mParent.get() : null;
        if (parent != null) {
            parent.mChildren.remove(this);
            mParent = null;
        }
        nativeRemoveFromParent(mNativeRef);
    }

    /**
     * Remove all children from this Node.
     */
    public void removeAllChildNodes(){
        List<Node> children = new ArrayList<Node>(mChildren);
        for (Node child : children) {
            child.removeFromParentNode();
        }
    }

    /**
     * Set the position of this Node. The position defines the Node's location within the coordinate
     * system of its parent. The default position is (0, 0, 0): the origin of the parent node’s
     * coordinate system.
     *
     * @param position The position as a {@link Vector}.
     */
    public void setPosition(Vector position){
        nativeSetPosition(mNativeRef, position.x, position.y, position.z);
    }

    /**
     * Get the real-time position of this Node. The real-time position is the Node's location in the
     * coordinate system of its parent, as it currently appears on-screen. Note this may not match
     * the value last set by {@link #setPosition(Vector)} because the real-time position is
     * influenced by many factors: animation, the physics simulation, and more.
     *
     * @return The real-time position as a {@link Vector}.
     */
    public Vector getPositionRealtime() {
        return new Vector(nativeGetPosition(mNativeRef));
    }

    /**
     * Set the orientation of this Node, expressed as three Euler angles in a {@link Vector}.
     * Specifically, the X component is rotation about the X axis (pitch), the Y component is
     * rotation about the node's Y axis (yaw), and Z is the rotation around the node's Z axis
     * (roll).
     * <p>
     * The rotation is made relative to the rotation pivot. See {@link
     * Node#setRotationPivot(Vector)}.
     *
     * @param rotation Vector containing the rotation as three Euler angles.
     */
    public void setRotation(Vector rotation) {
        nativeSetRotationEuler(mNativeRef, rotation.x, rotation.y, rotation.z);
    }

    /**
     * Set the orientation of this Node, expressed as a {@link Quaternion}.
     *
     * @param rotation The rotation in Quaternion form.
     */
    public void setRotation(Quaternion rotation) {
        nativeSetRotationQuaternion(mNativeRef, rotation.x, rotation.y, rotation.z, rotation.w);
    }

    /**
     * Get the <i>real-time</i> orientation of this Node, expressed as three Euler angles.
     * Specifically, the X component is rotation about the X axis (pitch), the Y component is
     * rotation about the node's Y axis (yaw), and Z is the rotation around the node's Z axis
     * (roll). This returns the real-time orientation, meaning the orientation of the Node as it
     * currently appears on-screen.
     * <p>
     * Note this may not match the value last set by {@link #setRotation(Vector)} because the
     * real-time rotation is influenced by many factors: animation, the physics simulation, and
     * more.
     *
     * @return The rotation in Euler form.
     */
    public Vector getRotationEulerRealtime() {
        return new Vector(nativeGetRotationEuler(mNativeRef));
    }

    /**
     * Get the <i>real-time</i> orientation of this Node, expressed as a {@link Quaternion}.  This
     * returns the real-time orientation, meaning the orientation of the Node as it currently
     * appears on-screen. Note this may not match the value last set by {@link #setRotation(Vector)}
     * because the real-time rotation is influenced by many factors: animation, the physics
     * simulation, and more.
     *
     * @return The rotation in Quaternion form.
     */
    public Quaternion getRotationQuaternionRealtime() {
        return new Quaternion(nativeGetRotationQuaternion(mNativeRef));
    }

    /**
     * Set the scale of this node as a {@link Vector}. The {@link Geometry} of this Node (and all of
     * its child Nodes), will have its dimension multiplied in each direction by the corresponding
     * component of this vector. For example, if scale is (5, 5, 1), the Geometry's width and height
     * would be increased 5x, but it's extent along the Z axis would remain constant.
     * <p>
     * The default scale is (1, 1, 1). Scale is applied relative to the scale pivot. See {@link
     * Node#setScalePivot(Vector)} .
     *
     * @param scale The scale {@link Vector}.
     */
    public void setScale(Vector scale) {
        nativeSetScale(mNativeRef, scale.x, scale.y, scale.z);
    }

    /**
     * Get the <i>real-time</i> scale of this Node.
     * <p>
     * Note this may not match the value last set by {@link #setScale(Vector)} because there may be
     * an ongoing animation.
     *
     * @return The scale {@link Vector}.
     */
    public Vector getScaleRealtime() {
        return new Vector(nativeGetScale(mNativeRef));
    }

    /**
     * Set the point on the Node about which rotation will occur. This defaults to the (0, 0, 0),
     * the origin of the Node's coordinate system.
     * <p>
     * For example, imagine this Node's Geometry is a unit box (e.g. length 1.0 in each dimension).
     * If the rotation pivot is (0, 0, 0) and we rotate about the Z axis, the box will spin about
     * its center. If the rotation pivot is (-0.5, 0, -0.5) and we rotate about the Z axis, the box
     * will spin around its far-left edge.
     *
     * @param pivot The pivot point as a {@link Vector}.
     */
    public void setRotationPivot(Vector pivot) {
        mRotationPivot = pivot;
        nativeSetRotationPivot(mNativeRef, pivot.x, pivot.y, pivot.z);
    }

    /**
     * Get the rotation pivot of this Node. The rotation pivot defines the point about which
     * rotation occurs.
     *
     * @return The pivot point as a {@link Vector}.
     */
    public Vector getRotationPivot() {
        return mRotationPivot;
    }

    /**
     * Set the point on the Node around which scaling will occur. This defaults to the (0, 0, 0),
     * the origin of the Node's coordinate system.
     * <p>
     * For example, imagine this Node's Geometry is a unit box (e.g. length 1.0 in each dimension).
     * If the scale pivot is (0, 0, 0) and we scale by (2.0, 2.0, 2.0), the box will grow equally in
     * each direction, remaining centered at (0, 0, 0) but with a new width, height, and length of
     * (2, 2, 2).
     * <p>
     * If, however, the scale pivot is (-0.5, -0.5, 0.5) and we scale by the same amount, the box
     * will only grow along the +X, +Y, and -Z dimensions (up, to the right, and away from the
     * user). Its new center will be (1, 1, 1), and it's new width, height, and length will be (2,
     * 2, 2).
     *
     * @param pivot The pivot point as a {@link Vector}.
     */
    public void setScalePivot(Vector pivot) {
        mScalePivot = pivot;
        nativeSetScalePivot(mNativeRef, pivot.x, pivot.y, pivot.z);
    }

    /**
     * Get the scale pivot of this Node. The scale pivot defines the point around which scaling
     * occurs.
     *
     * @return The pivot point as a {@link Vector}.
     */
    public Vector getScalePivot() {
        return mScalePivot;
    }

    /**
     * Set the opacity of this Node. A value of 0.0 means the contents of the Node are fully
     * transparent, and a value of 1.0 means the contents of the Node are fully opaque. This applies
     * to all the Node's children as well. A Node's final opacity is the opacity of its parents,
     * and its own opacity, multiplied together.
     *
     * @param opacity The opacity from 0.0 to 1.0.
     */
    public void setOpacity(float opacity) {
        mOpacity = opacity;
        nativeSetOpacity(mNativeRef, opacity);
    }

    /**
     * Get the opacity of this Node. A value of 0.0 means the contents fo the Node are fully
     * transparent, and a value of 1.0 means the contents of the Node are fully opaque.
     *
     * @return The opacity from 0.0 to 1.0.
     */
    public float getOpacity() {
        return mOpacity;
    }

    /**
     * Set to false to hide this Node, and all of its children. The default value is true.
     *
     * @param visible True to reveal the Node, false to hide.
     */
    public void setVisible(boolean visible) {
        mVisible = visible;
        nativeSetVisible(mNativeRef, visible);
    }

    /**
     * Return true if this Node's <tt>visible</tt> property is true. Note this does not return
     * whether the Node is *actually* visible in the user's current view; it only returns the value
     * of this property.
     *
     * @return The visible property of this Node.
     */
    public boolean isVisible() {
        return mVisible;
    }

    /**
     * Set the behavior of dragging if dragging is enabled on the Node's {@link EventDelegate}.
     *
     * @param dragType The {@link DragType} to use.
     */
    public void setDragType(DragType dragType) {
        mDragType = dragType;
        nativeSetDragType(mNativeRef, dragType.getStringValue());
    }

    /**
     * Get the dragging behavior for this Node.
     * @return The {@link DragType} for this Node.
     */
    public DragType getDragType() {
        return mDragType;
    }

    /**
     * Set the bit mask that determines what lights will illuminate this Node. This bit mask is
     * bitwise and-ed (&) with each Light's influenceBitMask. If the result is > 0, then the Light
     * will illuminate this Node. The default value is 0x1.
     *
     * @param bitMask The bit mask to set.
     */
    public void setLightReceivingBitMask(int bitMask) {
        mLightReceivingBitMask = bitMask;
        nativeSetLightReceivingBitMask(mNativeRef, bitMask);
    }

    /**
     * Get the light receiving bit mask, which determines which lights will illuminate this Node.
     *
     * @return The bit mask.
     */
    public int getLightReceivingBitMask() {
        return mLightReceivingBitMask;
    }

    /**
     * Set the bit mask that determines for which lights the contents of this Node will cast
     * shadows.  This bit mask is bitwise and-ed (&) with each Light's influenceBitMask. If the
     * result is > 0, then this Node will cast shadows from the light. The default value is 0x1.
     *
     * @param bitMask The bit mask to set.
     */
    public void setShadowCastingBitMask(int bitMask) {
        mShadowCastingBitMask = bitMask;
        nativeSetShadowCastingBitMask(mNativeRef, bitMask);
    }

    /**
     * Get the shadow casting bit mask, which determines for which lights this Node will cast
     * shadows.
     *
     * @return The bit mask.
     */
    public int getShadowCastingBitMask() {
        return mShadowCastingBitMask;
    }

    /**
     * True if onHover events should use the Geometry of this Node to determine if the user is
     * hovering over this Node. If false, the Node's axis-aligned bounding box will be used
     * instead. High accuracy gazing is more accurate but takes more processing power. The default
     * value is false.
     *
     * @param highAccuracyGaze True to enable high accuracy gazing.
     */
    public void setHighAccuracyGaze(boolean highAccuracyGaze) {
        mHighAccuracyGaze = highAccuracyGaze;
        nativeSetHighAccuracyGaze(mNativeRef, highAccuracyGaze);
    }

    /**
     * Returns true if high accuracy gaze is enabled.
     *
     * @return True if this Node is using high accuracy gazing.
     */
    public boolean getHighAccuracyGaze() {
        return mHighAccuracyGaze;
    }

    /**
     * When set to true, this Node will ignore events and not prevent Nodes behind it from
     * receiving event callbacks.
     * <p>
     * The default value is false.
     *
     * @param ignore True to make this Node ignore events.
     */
    public void setIgnoreEventHandling(boolean ignore) {
        mIgnoreEventHandling = ignore;
        nativeSetIgnoreEventHandling(mNativeRef, ignore);
    }

    /**
     * Returns true if this Node is currently set to ignore event handling.
     *
     * @return True if this Node is ignoring event handling.
     */
    public boolean getIgnoreEventHandling() {
        return mIgnoreEventHandling;
    }

    /**
     * @hide
     * @param hierarchicalRendering
     */
    public void setHierarchicalRendering(boolean hierarchicalRendering) {
        nativeSetHierarchicalRendering(mNativeRef, hierarchicalRendering);
    }

    /**
     * Set the {@link Geometry} of this Node. Geometries are the actual 3D objects that are rendered
     * for each Node.
     *
     * @param geometry The {@link Geometry} to attach to this Node. Null to remove any Geometry from
     *                 this Node.
     */
    public void setGeometry(Geometry geometry) {
        if (geometry != null) {
            nativeSetGeometry(mNativeRef, geometry.mNativeRef);
        }
        else {
            nativeClearGeometry(mNativeRef);
        }
        mGeometry = geometry;
    }

    /**
     * Get the {@link Geometry} attached to this Node. Geometries are the actual 3D objects that are
     * rendered for each Node.
     *
     * @return The Geometry attached to this Node, or null if no Geometry is attached.
     */
    public Geometry getGeometry() {
        return mGeometry;
    }

    /**
     * Attach the given {@link Camera} to this Node. To make this Camera active, this Node
     * must be set to the point of view via {@link Renderer#setPointOfView(Node)}.
     *
     * @param camera The Camera to add to this Node. Null to remove any set Camera.
     */
    public void setCamera(Camera camera) {
        mCamera = camera;
        if (camera == null) {
            nativeClearCamera(mNativeRef);
        }
        else {
            nativeSetCamera(mNativeRef, camera.mNativeRef);
        }
    }

    /**
     * Get the {@link Camera} attached to this Node.
     *
     * @return The {@link Camera}, or null if none is attached.
     */
    public Camera getCamera() {
        return mCamera;
    }

    /**
     * Set the {@link TransformBehavior}s to use for this Node. Transform behaviors
     * impact how a Node computes its position, rotation and scale.
     *
     * @param transformBehaviors The TransformBehaviors to set.
     */
    public void setTransformBehaviors(EnumSet<TransformBehavior> transformBehaviors) {
        mTransformBehaviors = transformBehaviors;

        String[] behaviors = new String[transformBehaviors.size()];
        int i = 0;
        for (TransformBehavior behavior : transformBehaviors) {
            behaviors[i] = behavior.getStringValue();
            ++i;
        }
        nativeSetTransformBehaviors(mNativeRef, behaviors);
    }

    /**
     * Return the {@link TransformBehavior}s used by this Node.
     *
     * @return The TransformBehaviors in an EnumSet.
     */
    public EnumSet<TransformBehavior> getTransformBehaviors() {
        return mTransformBehaviors;
    }

    /**
     * Set the tag for this Node. The tag is a non-unique identifier that you can use for any
     * purpose. Tags are also used in the physics simulation to identify objects in collisions.
     *
     * @param tag The tag to set, which is any arbitrary String.
     */
    public void setTag(String tag) {
        mTag = tag;
        nativeSetTag(mNativeRef, tag);
    }

    /**
     * Return the tag for this Node. The tag is a non-unique identifier that you can use for any
     * purpose.
     *
     * @return The tag used by this Node, or null if no tag is set.
     */
    public String getTag() {
        return mTag;
    }

    /**
     * Get the names of the animations available to run on this Node or any of its children.
     * If this Node represents a scene loaded from a 3D file format like FBX, then the returned
     * Set will contain the names of all skeletal and keyframe animations that were installed.
     *
     * @return The names of each animation in this Node.
     */
    public Set<String> getAnimationKeys() {
        return new HashSet<String>(Arrays.asList(nativeGetAnimationKeys(mNativeRef)));
    }

    /**
     * Create a {@link PhysicsBody} for this Node, which makes the Node participate in the physics
     * simulation. The PhysicsBody can be used to tune the way in which this Node responds to other
     * physics bodies, global forces, and collisions.
     *
     * @param rigidBodyType The rigid body type, which controls how the Node responds to forces and
     *                      collisions.
     * @param mass          The mass of the PhysicsBody, in kilograms.
     * @param shape         The shape of the PhysicsBody.
     * @return The initialized {@link PhysicsBody}.
     */
    public PhysicsBody initPhysicsBody(PhysicsBody.RigidBodyType rigidBodyType, float mass,
                                       PhysicsShape shape) {
        mPhysicsBody = new PhysicsBody(mNativeRef, rigidBodyType, mass, shape);
        return mPhysicsBody;
    }

    /**
     * Clear the {@link PhysicsBody} of this Node. This will stop the Node from participating
     * in the physics simulation.
     */
    public void clearPhysicsBody() {
        if (mPhysicsBody != null) {
            mPhysicsBody.clear();
        }
    }

    /**
     * Return true if this Node has a {@link PhysicsBody} attached, meaning its participates
     * in the physics simulation.
     * @return True if this Node participates in the physics simulation.
     */
    public boolean hasPhysics() {
        return mPhysicsBody != null;
    }

    /**
     * Get the {@link PhysicsBody} associated with this Node. The PhysicsBody can be used to tune
     * the way in which this Node responds to other physics bodies, global forces, and collisions.
     * Returns null if this Node is not participating in the physics simulation. To activate physics
     * for a node, use {@link #initPhysicsBody(PhysicsBody.RigidBodyType, float, PhysicsShape)}.
     *
     * @return The {@link PhysicsBody} associated with this Node.
     */
    public PhysicsBody getPhysicsBody() {
        return mPhysicsBody;
    }

    /*
     * JNI functions for view properties.
     */
    private native long nativeCreateNode();
    private native void nativeDestroyNode(long nodeReference);
    private native void nativeAddChildNode(long nodeReference, long childNodeReference);
    private native void nativeRemoveFromParent(long nodeReference);
    private native void nativeRemoveAllChildNodes(long nodeReference);
    private native void nativeSetHierarchicalRendering(long nodeReference, boolean hierarchicalRendering);
    private native void nativeSetGeometry(long nodeReference, long geoReference);
    private native void nativeClearGeometry(long nodeReference);
    private native void nativeSetPosition(long nodeReference, float x, float y, float z);
    private native void nativeSetRotationEuler(long nodeReference, float x, float y, float z);
    private native void nativeSetRotationQuaternion(long nodeReference, float x, float y, float z, float w);
    private native void nativeSetScale(long nodeReference, float x, float y, float z);
    private native void nativeSetRotationPivot(long nodeReference, float x, float y, float z);
    private native void nativeSetScalePivot(long nodeReference, float x, float y, float z);
    private native void nativeSetOpacity(long nodeReference, float opacity);
    private native void nativeSetVisible(long nodeReference, boolean visible);
    private native void nativeSetDragType(long nodeReference, String dragType);
    private native void nativeSetLightReceivingBitMask(long nodeReference, int bitMask);
    private native void nativeSetShadowCastingBitMask(long nodeReference, int bitMask);
    private native void nativeSetIgnoreEventHandling(long nodeReference, boolean visible);
    private native void nativeSetHighAccuracyGaze(long nodeReference, boolean enabled);
    private native void nativeSetTransformBehaviors(long nodeReference, String[] transformBehaviors);
    private native void nativeSetEventDelegate(long nodeReference, long eventDelegateRef);
    private native long nativeSetTransformDelegate(long nodeReference, double throttlingWindow);
    private native void nativeRemoveTransformDelegate(long nodeReference, long mNativeTransformDelegate);
    private native float[] nativeGetPosition(long nodeReference);
    private native float[] nativeGetScale(long nodeReference);
    private native float[] nativeGetRotationEuler(long nodeReference);
    private native float[] nativeGetRotationQuaternion(long nodeReference);
    private native String[] nativeGetAnimationKeys(long nodeReference);
    private native Geometry nativeGetGeometry(long nodeReference);
    private native void nativeSetTag(long nodeReference, String tag);
    private native void nativeSetCamera(long nodeReference, long cameraReference);
    private native void nativeClearCamera(long nodeReference);

    /*
     TransformDelegate Callback functions called from JNI
     */
    private WeakReference<TransformDelegate> mTransformDelegate = null;
    private long mNativeTransformDelegate = INVALID_REF;

    /**
     * The TransformDelegate receives callbacks when the Node's position has changed.
     */
    public interface TransformDelegate {

        /**
         * Invoked when this Node's position has been updated. The incoming position is the
         * location of the Node in the coordinate system of its parent.
         *
         * @param position The updated position as a {@link Vector}.
         */
        void onPositionUpdate(Vector position);
    }

    /**
     * @hide
     * @param x
     * @param y
     * @param z
     */
    public final void onPositionUpdate(float x, float y, float z) {
        if (mTransformDelegate.get() != null){
            mTransformDelegate.get().onPositionUpdate(new Vector(x,y,z));
        }
    }

    /**
     * Set the {@link TransformDelegate} for this Node. The delegate will receive callbacks
     * when the Node's position has changed.
     *
     * @param transformDelegate The delegate to set.
     * @param distanceFilter    The delegate will be notified whenever the Node's position has
     *                          changed by at least this magnitude.
     */
    public void setTransformDelegate(TransformDelegate transformDelegate, double distanceFilter) {
        if (mNativeTransformDelegate == INVALID_REF) {
            mNativeTransformDelegate = nativeSetTransformDelegate(mNativeRef, distanceFilter);
        }
        mTransformDelegate = new WeakReference<TransformDelegate>(transformDelegate);
    }

    /**
     * Remove any {@link TransformDelegate} that is currently attached to this Node.
     */
    public void removeTransformDelegate() {
        if (mNativeTransformDelegate != INVALID_REF) {
            mTransformDelegate = null;
            nativeRemoveTransformDelegate(mNativeRef, mNativeTransformDelegate);
            mNativeTransformDelegate = INVALID_REF;
        }
    }
}
