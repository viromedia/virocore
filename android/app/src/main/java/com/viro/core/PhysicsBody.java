/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.core;

import java.lang.ref.WeakReference;

/**
 * PhysicsBody encapsulates the physics simulation of a particular {@link Node}. The physics
 * simulation includes computing the effects of gravity, friction, and user-supplied forces. The
 * simulation is updated each frame prior to rendering. In order to add physics to a Node, invoke
 * {@link Node#initPhysicsBody(RigidBodyType, float, PhysicsShape)}.
 */
public class PhysicsBody {

    /**
     * The RigidBodyType of a PhysicsBody controls how the body responds to forces and collisions.
     */
    public enum RigidBodyType {
        /**
         * Dynamic rigid bodies are fully simulated physics objects. They are affected by forces and
         * collisions, and are the most performance-intensive physics body.
         */
        DYNAMIC("dynamic"),

        /**
         * Kinematic rigid bodies are not themselves affected by forces and collisions (they are
         * manually moved) but they are able to cause cause collisions. They are moved manually. For
         * example, use a Kinematic body to simulate a user pushing objects with her finger, or to
         * simulate projectiles like bullets.
         */
        KINEMATIC("kinematic"),

        /**
         * Static rigid bodies do not move; other objects can collide with them but they do not
         * otherwise participate in the physics simulation. Examples include walls, floor, and other
         * other fixtures in your scene.
         */
        STATIC("static");

        private String mStringValue;
        private RigidBodyType(String value) {
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

    private long mNativeRef;
    private RigidBodyType mRigidBodyType;
    private PhysicsShape mShape;
    private float mMass;
    private float mFriction;
    private float mRestitution;
    private boolean mEnabled;
    private boolean mUseGravity;
    private Vector mVelocity;

    /**
     * Create a new PhysicsBody.
     * @hide
     */
    public PhysicsBody(long nativeRef, RigidBodyType rigidBodyType, float mass,
                       PhysicsShape shape) {
        mNativeRef = nativeRef;
        mRigidBodyType = rigidBodyType;
        mShape = shape;
        mEnabled = true;
        mFriction = 0.5f; // matches btRigidBody.h default
        mRestitution = 0.0f; // matches btRigidBody.h default
        mUseGravity = true;
        nativeInitPhysicsBody(nativeRef, rigidBodyType.getStringValue(), mass,
                shape != null ? shape.getType() : null,
                shape != null ? shape.getParams() : null);
    }

    /**
     * Removes the PhysicsBody from the node.
     * @hide
     */
    public void clear() {
        nativeClearPhysicsBody(mNativeRef);
    }

    /**
     * Set the {@link PhysicsShape} that should be used to approximate the PhysicsBody during the
     * physics simulation.
     *
     * @param shape The solid volume of the PhysicsBody.
     */
    public void setShape(PhysicsShape shape) {
        mShape = shape;
        nativeSetPhysicsShape(mNativeRef, shape.getType(), shape.getParams());
    }

    /**
     * Get the {@link PhysicsShape} that is used to approximate the PhysicsBody.
     *
     * @return The solid volume of the PhysicsBody.
     */
    public PhysicsShape getShape() {
        return mShape;
    }

    /**
     * The mass of the PhysicsBody in kilograms. Determines how the body is impacted by forces and
     * collisions with other bodies. Only used for {@link RigidBodyType#DYNAMIC} bodies.
     *
     * @param mass The mass in kg.
     */
    public void setMass(float mass) {
        mMass = mass;
        nativeSetPhysicsMass(mNativeRef, mass);
    }

    /**
     * Return the mass of this PhysicsBody in kg.
     *
     * @return The mass in kg.
     */
    public float getMass() {
        return mMass;
    }

    /**
     * Set the moment of inertia for the PhysicsBody. The moment of inertia determines how the body
     * responds to torque. The given vector indicates the moment of inertia for each axis, X, Y, and
     * Z. By default, the moment of inertia is automatically determined by the shape and mass used.
     *
     * @param momentOfInertia The moment of inertia along the X, Y, and Z axes.
     */
    public void setMomentOfInertia(Vector momentOfInertia) {
        nativeSetPhysicsInertia(mNativeRef, momentOfInertia.toArray());
    }

    /**
     * Set the friction of the PhysicsBody, between 0.0 and 1.0. Friction determines the body's
     * resistance to sliding motion, effectively simulating the "roughness" of a surface. If two
     * sliding bodies each have a value of 0, then they will slide completely freely against one
     * another; if they both have a value of 1.0, then the bodies will not slide at all against one
     * another.
     * <p>
     * Defaults to 0.5.
     *
     * @param friction The friction value between 0.0 and 1.0.
     */
    public void setFriction(float friction) {
        mFriction = friction;
        nativeSetPhysicsFriction(mNativeRef, friction);
    }

    /**
     * Get the friction value for this PhysicsBody.
     *
     * @return The friction, between 0.0 and 1.0.
     */
    public float getFriction() {
        return mFriction;
    }

    /**
     * Set the restitution of the PhysicsBody. Restitution controls how "bouncy" an object or
     * surface is. When two objects collide, some of the energy of the collision is used to deform
     * the collided objects, some is used rebound the objects from one another, and some is lost to
     * heat. Restitution is a measure of how much of that kinetic energy is used for objects to
     * rebound from one another. To take a real-world example, a massive bowling bowl has a low
     * coefficient of restitution whereas a basketball has a high coefficient of restitution.
     * <p>
     * Restitution of 1.0 implies a perfectly elastic collision: the ball will bounce right back up
     * to its initial height. Restitution of 0.0 represents a completely inelastic collision: the
     * ball will stick to the floor after colliding.
     * <p>
     * Defaults to 0.0.
     *
     * @param restitution The restitution value between 0.0 and 1.0.
     */
    public void setRestitution(float restitution) {
        mRestitution = restitution;
        nativeSetPhysicsRestitution(mNativeRef, restitution);
    }

    /**
     * Get the restitution value of this PhysicsBody.
     *
     * @return The restitution value.
     */
    public float getRestitution() {
        return mRestitution;
    }

    /**
     * The enabled flag is used to enable or disable the physics simulation for this PhysicsBody.
     * If false, the Node will not be impacted by the physics simulation.
     *
     * @param enabled True to enable the physics simulation.
     */
    public void setEnabled(boolean enabled) {
        mEnabled = enabled;
        nativeSetPhysicsEnabled(mNativeRef, enabled);
    }

    /**
     * Return true if the physics simulation is enabled for this PhysicsBody.
     *
     * @return True if enabled.
     */
    public boolean isEnabled() {
        return mEnabled;
    }

    /**
     * Set to true to make this PhysicsBody respond to gravity. Gravity globally accelerates physics
     * bodies in a specific direction. It can be set in the {@link PhysicsWorld}.
     * <p>
     * Defaults to true.
     *
     * @param useGravity True to use gravity.
     */
    public void setUseGravity(boolean useGravity) {
        nativeSetPhysicsUseGravity(mNativeRef, useGravity);
    }

    /**
     * Return true if this PhysicsBody responds to gravity.
     *
     * @return True if this PhysicsBody uses gravity.
     */
    public boolean getUseGravity() {
        return mUseGravity;
    }

    /**
     * Set the velocity of this PhysicsBody, independent from all forces. Used to directly move an
     * object without applying a force. Units are m/s. Invoking this method  will override any
     * forces that are already applied on this PhysicsBody.
     *
     * @param velocity   The velocity to set.
     * @param isConstant True if this velocity should be applied every frame. False if this should
     *                   be an instantaneous application of the velocity for only one frame (e.g.
     *                   like an impulse).
     */
    public void setVelocity(Vector velocity, boolean isConstant) {
        if (isConstant) {
            mVelocity = velocity;
        }
        nativeSetPhysicsVelocity(mNativeRef, velocity.toArray(), isConstant);
    }

    /**
     * Get the velocity of this PhysicsBody. Note this only returns the last applied *constant*
     * velocity of this PhysicsBody, as set by {@link #setVelocity(Vector, boolean)}. It does not
     * return the real-time actual velocity of the PhysicsBody.
     *
     * @return The last applied constant velocity.
     */
    public Vector getVelocity() {
        return mVelocity;
    }

    /**
     * Clear any constant forces that were applied to this PhysicsBody.
     */
    public void clearForce() {
        nativeClearPhysicsForce(mNativeRef);
    }

    /**
     * Apply a constant force to this PhysicsBody. Constant forces can be useful in situations for
     * moving objects in constant motion; for example, when adding thrust to a spaceship being
     * launched into the sky. For each force, you can specify both its
     * magnitude (in newtons) and its location (on the object).
     *
     * @param force    The magnitude of the force along each principal axis (X, Y, and Z).
     * @param position The location in the PhysicsBody's local coordinate system at which the force
     *                 is applied.
     */
    public void applyForce(Vector force, Vector position) {
        nativeApplyPhysicsForce(mNativeRef, force.toArray(), position.toArray());
    }

    /**
     * Apply a constant rotational force to this PhysicsBody. Applying torque to a body changes its
     * angular velocity, rotating it without changing its linear acceleration. The torque vector is
     * specified as a magnitude along each principal axis. Applying a torque of {0.0, 0.0, 1.0}
     * causes the PhysicsBody to spin counterclockwise around the Z-axis.
     *
     * @param torque The magnitude of the torque along each principal axis (X, Y, and Z).
     */
    public void applyTorque(Vector torque) {
        nativeApplyPhysicsTorque(mNativeRef, torque.toArray());
    }

    /**
     * Apply an instantaneous force to this PhysicsBody. Impulse forces are useful in situations
     * where you would like to apply an instantaneous burst of force to an object. These are often
     * useful for launching projectile objects.
     *
     * @param impulse  The magnitude of the impulse along each principal axis (X, Y, and Z).
     * @param position The location in the PhysicsBody's local coordinate system at which the
     *                 impulse is applied.
     */
    public void applyImpulse(Vector impulse, Vector position){
        nativeApplyPhysicsImpulse(mNativeRef, impulse.toArray(), position.toArray());
    }

    /**
     * Apply an instantaneous rotational force to this PhysicsBody.
     *
     * @param torque The magnitude of the torque impulse along each principal axis (X, Y, and Z).
     */
    public void applyTorqueImpulse(Vector torque) {
        nativeApplyPhysicsTorqueImpulse(mNativeRef, torque.toArray());
    }

    /**
     * @hide
     * @param bodyType
     * @param mass
     * @return
     */
    public static String checkIsValidBodyType(String bodyType, float mass) {
        return nativeIsValidBodyType(bodyType, mass);
    }

    /**
     * @hide
     * @param shapeType
     * @param params
     * @return
     */
    public static String checkIsValidShapeType(String shapeType, float params[]) {
        return nativeIsValidShapeType(shapeType, params);
    }

    /*
     * Collision listener callback.
     */
    private WeakReference<CollisionListener> mCollisionListener = null;
    private static long INVALID_REF = Long.MAX_VALUE;
    private long mNativePhysicsDelegate = INVALID_REF;

    /**
     * Set the {@link CollisionListener} to use for this PhysicsBody. The listener receives a callback
     * each time this PhysicsBody collides with another.
     *
     * @param listener The listener to use. Null to remove the listener.
     */
    public void setCollisionListener(CollisionListener listener) {
        if (listener != null && mNativePhysicsDelegate == INVALID_REF) {
            mCollisionListener = new WeakReference<CollisionListener>(listener);
            mNativePhysicsDelegate = nativeSetPhysicsDelegate(mNativeRef);
        }
        else if (listener == null && mNativePhysicsDelegate != INVALID_REF) {
            nativeClearPhysicsDelegate(mNativeRef, mNativePhysicsDelegate);
            mNativePhysicsDelegate = INVALID_REF;
            mCollisionListener = null;
        }
    }

    /**
     * Callback interface for responding to PhysicsBody collisions.
     */
    public interface CollisionListener {

        /**
         * Callback invoked each time this listener's PhysicsBody enters a collision with another
         * PhysicsBody.
         *
         * @param tag      The tag of the {@link Node} associated with the collided PhysicsBody.
         * @param position The position on this listener's PhysicsBody where the collision occurred.
         * @param normal   The normal vector on the surface of this listener's PhysicsBody where the
         *                 collision occurred.
         */
        void onCollided(String tag, Vector position, Vector normal);
    }

    /**
     * @hide
     * @param collidedTag
     * @param posX
     * @param posY
     * @param posZ
     * @param normX
     * @param normY
     * @param normZ
     */
    public void onCollided(String collidedTag,
                           float posX, float posY, float posZ,
                           float normX, float normY, float normZ) {
        if (mCollisionListener != null && mCollisionListener.get() != null
                && mNativePhysicsDelegate != INVALID_REF) {
            Vector position = new Vector(posX, posY, posZ);
            Vector normal = new Vector(normX, normY, normZ);
            mCollisionListener.get().onCollided(collidedTag, position, normal);
        }
    }

    private native void nativeInitPhysicsBody(long nodeReference, String rigidBodyType,
                                              float mass, String shapeType, float shapeParams[]);
    private native void nativeClearPhysicsBody(long nodeReference);
    private native void nativeSetPhysicsShape(long nodeReference, String type, float[] params);
    private native void nativeSetPhysicsMass(long nodeReference, float mass);
    private native void nativeSetPhysicsInertia(long nodeReference, float[] inertia);
    private native void nativeSetPhysicsFriction(long nodeReference, float friction);
    private native void nativeSetPhysicsRestitution(long nodeReference, float restitution);
    private native void nativeSetPhysicsEnabled(long nodeReference, boolean enabled);
    private native void nativeSetPhysicsUseGravity(long nodeReference, boolean useGravity);
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
    private native void nativeSetPhysicsVelocity(long nodeReference, float[] velocity, boolean isConstant);

}
