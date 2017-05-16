/**
 * Copyright © 2016 Viro Media. All rights reserved.
 */
package com.viro.renderer.jni;

import android.content.Context;
import java.util.List;

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
        if (position.length != 3){
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

    public void setOpacity(float opacity){
        nativeSetOpacity(mNativeRef, opacity);
    }

    public void setVisible(boolean visible){
        nativeSetVisible(mNativeRef, visible);
    }

    public void setHighAccuracyGaze(boolean visible){
        nativeSetHighAccuracyGaze(mNativeRef, visible);
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

    public String checkIsValidBodyType(String bodyType, float mass){
        return nativeIsValidBodyType(bodyType, mass);
    }

    public String checkIsValidShapeType(String shapeType, float params[]){
        return nativeIsValidShapeType(shapeType, params);
    }

    private native long nativeCreateNode();
    private native void nativeDestroyNode(long nodeReference);
    private native void nativeAddChildNode(long nodeReference, long childNodeReference);
    private native void nativeRemoveFromParent(long nodeReference);
    private native void nativeSetHierarchicalRendering(long nodeReference, boolean hierarchicalRendering);
    private native void nativeSetPosition(long nodeReference, float x, float y, float z);
    private native void nativeSetRotation(long nodeReference, float x, float y, float z);
    private native void nativeSetScale(long nodeReference, float x, float y, float z);
    private native void nativeSetOpacity(long nodeReference, float opacity);
    private native void nativeSetVisible(long nodeReference, boolean visible);
    private native void nativeSetHighAccuracyGaze(long nodeReference, boolean enabled);
    private native void nativeSetMaterials(long nodeReference, long[] materials);
    private native void nativeSetTransformBehaviors(long nodeReference, String[] transformBehaviors);
    private native void nativeSetEventDelegate(long nodeReference, long eventDelegateRef);

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
    private native String nativeIsValidBodyType(String bodyType, float mass);
    private native String nativeIsValidShapeType(String shapeType, float[] shapeParams);
}
