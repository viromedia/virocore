/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import java.lang.ref.WeakReference;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * Java JNI wrapper for linking the following classes below across the bridge.
 *
 * Java JNI Wrapper     : com.viro.renderer.NodeJni.java
 * Cpp JNI wrapper      : Node_JNI.cpp
 * Cpp Object           : VRONode.cpp
 */
public class NodeJni {
    final protected long mNativeRef;

    protected boolean mDestroyed = false;
    private EventDelegateJni mEventDelegate = null;

    public NodeJni() {
        mNativeRef = nativeCreateNode();
    }

    public void destroy() {
        mDestroyed = true;
        removeTransformDelegate();
        nativeDestroyNode(mNativeRef);
    }

    public void setEventDelegateJni(EventDelegateJni eventDelegate){
        if (mEventDelegate != null){
            mEventDelegate.destroy();
        }

        mEventDelegate = eventDelegate;

        if (mEventDelegate != null){
            nativeSetEventDelegate(mNativeRef, mEventDelegate.mNativeRef);
        }
    }

    public void addChildNode(NodeJni childNode){
        nativeAddChildNode(mNativeRef, childNode.mNativeRef);
    }

    public void removeChildNode(NodeJni childNode){
        nativeRemoveFromParent(childNode.mNativeRef);
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

    public void setHighAccuracyGaze(boolean visible){
        nativeSetHighAccuracyGaze(mNativeRef, visible);
    }

    public void setIgnoreEventHandling(boolean ignore){
        nativeSetIgnoreEventHandling(mNativeRef, ignore);
    }

    public void setHierarchicalRendering(boolean hierarchicalRendering) {
        nativeSetHierarchicalRendering(mNativeRef, hierarchicalRendering);
    }

    public void setGeometry(BaseGeometry geometry){
        if (geometry == null){
            throw new IllegalArgumentException("Missing Required geometry to be set on Node.");
        }
        // Reverse the setting of node to occur within the
        // corresponding geometry to avoid polymorphing across
        // the JNI layer.
        geometry.attachToNode(this);
    }

    public boolean setMaterials(List<MaterialJni> materials) {
        // Create list of longs (refs) to all the materials. If any
        // material has already been destroyed, return false
        long[] materialRefs = new long[materials.size()];
        for (int i = 0; i < materials.size(); i++) {
            materialRefs[i] = materials.get(i).mNativeRef;
            if (materialRefs[i] == 0) {
                return false;
            }
        }
        nativeSetMaterials(mNativeRef, materialRefs);
        return true;
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

    /**
     * JNI functions for view properties.
     */
    private native long nativeCreateNode();
    private native void nativeDestroyNode(long nodeReference);
    private native void nativeAddChildNode(long nodeReference, long childNodeReference);
    private native void nativeRemoveFromParent(long nodeReference);
    private native void nativeSetHierarchicalRendering(long nodeReference, boolean hierarchicalRendering);
    private native void nativeSetPosition(long nodeReference, float x, float y, float z);
    private native void nativeSetRotation(long nodeReference, float x, float y, float z);
    private native void nativeSetScale(long nodeReference, float x, float y, float z);
    private native void nativeSetRotationPivot(long nodeReference, float x, float y, float z);
    private native void nativeSetScalePivot(long nodeReference, float x, float y, float z);
    private native void nativeSetOpacity(long nodeReference, float opacity);
    private native void nativeSetVisible(long nodeReference, boolean visible);
    private native void nativeSetIgnoreEventHandling(long nodeReference, boolean visible);
    private native void nativeSetHighAccuracyGaze(long nodeReference, boolean enabled);
    private native void nativeSetMaterials(long nodeReference, long[] materials);
    private native void nativeSetTransformBehaviors(long nodeReference, String[] transformBehaviors);
    private native void nativeSetEventDelegate(long nodeReference, long eventDelegateRef);
    private native long nativeSetTransformDelegate(long nodeReference, double throttlingWindow);
    private native void nativeRemoveTransformDelegate(long nodeReference, long mNativeTransformDelegate);
    private native String[] nativeGetAnimationKeys(long nodeReference);

    /**
     * TransformDelegate Callback functions called from JNI
     */
    private WeakReference<TransformDelegate> mTransformDelegate = null;
    private long mNativeTransformDelegate = INVALID_REF;
    public interface TransformDelegate{
        void onPositionUpdate(float[] position);
    }

    public void onPositionUpdate(float x, float y, float z){
        if (mTransformDelegate.get() != null){
            mTransformDelegate.get().onPositionUpdate(new float[]{x,y,z});
        }
    }

    public void setTransformDelegate(TransformDelegate transformDelegate, double distanceFilter){
        if (mNativeTransformDelegate == INVALID_REF){
            mNativeTransformDelegate = nativeSetTransformDelegate(mNativeRef, distanceFilter);
        }
        mTransformDelegate = new WeakReference<TransformDelegate>(transformDelegate);
    }

    public void removeTransformDelegate(){
        if (mNativeTransformDelegate != INVALID_REF){
            mTransformDelegate = null;
            nativeRemoveTransformDelegate(mNativeRef, mNativeTransformDelegate);
            mNativeTransformDelegate = INVALID_REF;
        }
    }

    /**
     * Physics properties
     */
    public void initPhysicsBody(String rigidBodyType, float mass,
                                String shapeType, float shapeParams[]){
        nativeInitPhysicsBody(mNativeRef, rigidBodyType, mass, shapeType, shapeParams);
    }

    public void clearPhysicsBody(){
        nativeClearPhysicsBody(mNativeRef);
    }

    public void setPhysicsShape(String shapeType, float[] params){
        nativeSetPhysicsShape(mNativeRef, shapeType, params);
    }

    public void setPhysicsMass(float mass){
        nativeSetPhysicsMass(mNativeRef, mass);
    }

    public void setPhysicsInertia(float[] inertia){
        nativeSetPhysicsInertia(mNativeRef, inertia);
    }

    public void setPhysicsFriction(float friction){
        nativeSetPhysicsFriction(mNativeRef, friction);
    }

    public void setPhysicsRestitution(float restitution){
        nativeSetPhysicsRestitution(mNativeRef, restitution);
    }

    public void setPhysicsEnabled(boolean enabled){
        nativeSetPhysicsEnabled(mNativeRef, enabled);
    }

    public void setPhsyicsUseGravity(boolean useGravity){
        nativeSetPhsyicsUseGravity(mNativeRef, useGravity);
    }

    public static String checkIsValidBodyType(String bodyType, float mass){
        return nativeIsValidBodyType(bodyType, mass);
    }

    public static String checkIsValidShapeType(String shapeType, float params[]){
        return nativeIsValidShapeType(shapeType, params);
    }

    public void clearPhysicsForce(){
        nativeClearPhysicsForce(mNativeRef);
    }

    public void applyPhysicsForce(float[] force, float[] position){
        nativeApplyPhysicsForce(mNativeRef, force, position);
    }

    public void applyPhysicsTorque(float[] torque){
        nativeApplyPhysicsTorque(mNativeRef, torque);
    }

    public void applyPhysicsImpulse(float[] force, float[] position){
        nativeApplyPhysicsImpulse(mNativeRef, force, position);
    }

    public void applyPhysicsTorqueImpulse(float[] torque){
        nativeApplyPhysicsTorqueImpulse(mNativeRef, torque);
    }

    public void setPhysicsVelocity(float[] velocity, boolean isConstant){
        nativeSetPhysicsVelocity(mNativeRef, velocity, isConstant);
    }

    /**
     * Physics Delegate callback.
     */
    private WeakReference<PhysicsDelegate> mPhysicsDelegate = null;
    private static long INVALID_REF = Long.MAX_VALUE;
    private long mNativePhysicsDelegate = INVALID_REF;

    public void setPhysicsDelegate(PhysicsDelegate delegate){
        if (delegate != null && mNativePhysicsDelegate == INVALID_REF) {
            mPhysicsDelegate = new WeakReference<PhysicsDelegate>(delegate);
            mNativePhysicsDelegate = nativeSetPhysicsDelegate(mNativeRef);
        } else if (delegate == null && mNativePhysicsDelegate != INVALID_REF){
            nativeClearPhysicsDelegate(mNativeRef, mNativePhysicsDelegate);
            mNativePhysicsDelegate = INVALID_REF;
            mPhysicsDelegate = null;
        }
    }

    public interface PhysicsDelegate {
        void onCollided(String tag, float[] position, float[] t);
    }

    public void onCollided(String collidedTag,
                           float posX, float posY, float posZ,
                           float normX, float normY, float normZ) {
        if (mPhysicsDelegate != null && mPhysicsDelegate.get() != null
                && mNativePhysicsDelegate != INVALID_REF) {
            float[] pos = {posX, posY, posZ};
            float[] norm = {normX, normY, normZ};
            mPhysicsDelegate.get().onCollided(collidedTag, pos, norm);
        }
    }

    /**
      * Native JNI Functions for physics
      */
    private native void nativeInitPhysicsBody(long nodeReference, String rigidBodyType,
                                              float mass, String shapeType, float shapeParams[]);
    private native void nativeClearPhysicsBody(long nodeReference);
    private native void nativeSetPhysicsShape(long nodeReference, String type, float[] params);
    private native void nativeSetPhysicsMass(long nodeReference, float mass);
    private native void nativeSetPhysicsInertia(long nodeReference, float[] inertia);
    private native void nativeSetPhysicsFriction(long nodeReference, float friction);
    private native void nativeSetPhysicsRestitution(long nodeReference, float restitution);
    private native void nativeSetPhysicsEnabled(long nodeReference, boolean enabled);
    private native void nativeSetPhsyicsUseGravity(long nodeReference, boolean useGravity);
    private native static String nativeIsValidBodyType(String bodyType, float mass);
    private native static String nativeIsValidShapeType(String shapeType, float[] shapeParams);
    private native void nativeApplyPhysicsForce(long nodeReference, float[] force,
                                                float[] position);
    private native void nativeApplyPhysicsTorque(long nodeReference, float[] torque);
    private native void nativeApplyPhysicsImpulse(long nodeReference, float[] force,
                                                       float[] position);
    private native void nativeApplyPhysicsTorqueImpulse(long nodeReference, float[] torque);
    private native void nativeClearPhysicsForce(long nodeReference);
    private native long nativeSetPhysicsDelegate(long nodeReference);
    private native void nativeClearPhysicsDelegate(long nodeReference, long delegateRef);
    private native void nativeSetTag(long nodeReference, String tag);
    private native void nativeSetPhysicsVelocity(long nodeReference, float[] velocity, boolean isConstant);
}
