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
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;


public class Node {

    private static long INVALID_REF = Long.MAX_VALUE;
    protected long mNativeRef;

    protected boolean mDestroyed = false;
    private EventDelegate mEventDelegate = null;
    private Geometry mGeometry;
    private PhysicsBody mPhysicsBody;

    public Node() {
        mNativeRef = nativeCreateNode();
    }

    /*
     This constructor to be called by child classes that want to
     override mNativeRef
     */
    protected Node(boolean dummyArg) {
        // no-op
    }

    /*
     Function called by child classes to set mNativeRef
     */
    protected void setNativeRef(long nativeRef) {
        mNativeRef = nativeRef;
    }

    public void dispose() {
        mDestroyed = true;
        removeTransformDelegate();

        if (mNativeRef != 0) {
            nativeDestroyNode(mNativeRef);
            mNativeRef = 0;
        }
    }

    public void setEventDelegateJni(EventDelegate eventDelegate){
        if (mEventDelegate != null) {
            mEventDelegate.destroy();
        }

        mEventDelegate = eventDelegate;
        if (mEventDelegate != null) {
            nativeSetEventDelegate(mNativeRef, mEventDelegate.mNativeRef);
        }
    }

    public void addChildNode(Node childNode){
        nativeAddChildNode(mNativeRef, childNode.mNativeRef);
    }

    public void removeChildNode(Node childNode){
        nativeRemoveFromParent(childNode.mNativeRef);
    }

    public void removeAllChildNodes(){
        nativeRemoveAllChildNodes(mNativeRef);
    }

    public void setPosition(float[] position){
        if (position.length < 3){
            throw new IllegalArgumentException("Missing a position coordinate: All three coordinates are needed [x,y,z]");
        }

        nativeSetPosition(mNativeRef, position[0], position[1], position[2]);
    }

    public void setRotation(float[] rotation){
        if (rotation.length != 3){
            throw new IllegalArgumentException("Missing a rotation coordinate: All three coordinates are needed [x,y,z]");
        }
        nativeSetRotation(mNativeRef, rotation[0], rotation[1], rotation[2]);
    }

    public void setScale(float[] scale){
        if (scale.length != 3){
            throw new IllegalArgumentException("Missing a scale coordinate: All three coordinates are needed [x,y,z]");
        }
        nativeSetScale(mNativeRef, scale[0], scale[1], scale[2]);
    }

    public void setRotationPivot(float[] pivot){
        if (pivot.length != 3){
            throw new IllegalArgumentException("Missing a pivot coordinate: All three coordinates are needed [x,y,z]");
        }
        nativeSetRotationPivot(mNativeRef, pivot[0], pivot[1], pivot[2]);
    }

    public void setScalePivot(float[] pivot){
        if (pivot.length != 3){
            throw new IllegalArgumentException("Missing a pivot coordinate: All three coordinates are needed [x,y,z]");
        }
        nativeSetScalePivot(mNativeRef, pivot[0], pivot[1], pivot[2]);
    }

    public void setOpacity(float opacity){
        nativeSetOpacity(mNativeRef, opacity);
    }

    public void setVisible(boolean visible){
        nativeSetVisible(mNativeRef, visible);
    }

    public void setDragType(String dragType) {
        nativeSetDragType(mNativeRef, dragType);
    }

    public void setLightReceivingBitMask(int bitMask) { nativeSetLightReceivingBitMask(mNativeRef, bitMask); }

    public void setShadowCastingBitMask(int bitMask) { nativeSetShadowCastingBitMask(mNativeRef, bitMask); }

    public void setHighAccuracyGaze(boolean visible) {
        nativeSetHighAccuracyGaze(mNativeRef, visible);
    }

    public void setIgnoreEventHandling(boolean ignore) {
        nativeSetIgnoreEventHandling(mNativeRef, ignore);
    }

    public void setHierarchicalRendering(boolean hierarchicalRendering) {
        nativeSetHierarchicalRendering(mNativeRef, hierarchicalRendering);
    }

    // TODO Figure out how to store a UI thread copy of the Geometry. Maybe the attachToNode
    //      returns a reference that I can store here?
    public void setGeometry(Geometry geometry){
        if (geometry != null) {
            nativeSetGeometry(mNativeRef, geometry.mNativeRef);
        }
        else {
            nativeClearGeometry(mNativeRef);
        }
        mGeometry = geometry;
    }

    public Geometry getGeometry() {
        return mGeometry;
    }

    public void setTransformBehaviors(String[] transformBehaviors) {
        nativeSetTransformBehaviors(mNativeRef, transformBehaviors);
    }

    public void setTag(String tag){
        nativeSetTag(mNativeRef, tag);
    }

    public Set<String> getAnimationKeys() {
        return new HashSet<String>(Arrays.asList(nativeGetAnimationKeys(mNativeRef)));
    }

    public PhysicsBody initPhysicsBody(PhysicsBody.RigidBodyType rigidBodyType, float mass,
                                       PhysicsShape shape) {
        mPhysicsBody = new PhysicsBody(mNativeRef, rigidBodyType, mass, shape);
        return mPhysicsBody;
    }

    public void clearPhysicsBody() {
        if (mPhysicsBody != null) {
            mPhysicsBody.clear();
        }
    }

    public boolean hasPhysics() {
        return mPhysicsBody != null;
    }

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
    private native void nativeSetRotation(long nodeReference, float x, float y, float z);
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
    private native String[] nativeGetAnimationKeys(long nodeReference);
    private native Geometry nativeGetGeometry(long nodeReference);
    private native void nativeSetTag(long nodeReference, String tag);

    /*
     * TransformDelegate Callback functions called from JNI
     */
    private WeakReference<TransformDelegate> mTransformDelegate = null;
    private long mNativeTransformDelegate = INVALID_REF;
    public interface TransformDelegate{
        void onPositionUpdate(float[] position);
    }

    public void onPositionUpdate(float x, float y, float z) {
        if (mTransformDelegate.get() != null){
            mTransformDelegate.get().onPositionUpdate(new float[]{x,y,z});
        }
    }

    public void setTransformDelegate(TransformDelegate transformDelegate, double distanceFilter) {
        if (mNativeTransformDelegate == INVALID_REF){
            mNativeTransformDelegate = nativeSetTransformDelegate(mNativeRef, distanceFilter);
        }
        mTransformDelegate = new WeakReference<TransformDelegate>(transformDelegate);
    }

    public void removeTransformDelegate(){
        if (mNativeTransformDelegate != INVALID_REF) {
            mTransformDelegate = null;
            nativeRemoveTransformDelegate(mNativeRef, mNativeTransformDelegate);
            mNativeTransformDelegate = INVALID_REF;
        }
    }
}
