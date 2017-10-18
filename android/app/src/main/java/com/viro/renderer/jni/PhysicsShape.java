package com.viro.renderer.jni;

/**
 * The physics simulation approximates physics bodies with simplified shapes. These shapes are
 * approximations for performance reasons.
 */
public interface PhysicsShape {
    /**
     * @hide
     * @return
     */
    String getType();
    /**
     * @hide
     * @return
     */
    float[] getParams();
}