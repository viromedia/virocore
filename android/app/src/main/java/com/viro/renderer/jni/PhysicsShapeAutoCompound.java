package com.viro.renderer.jni;

/**
 * AutoCompound shapes are automatically generated from a Node by approximating its geometry
 * as a composite of bounding boxes.
 */
public class PhysicsShapeAutoCompound implements PhysicsShape {

    /**
     * Construct a new AutoCompound Shape.
     */
    public PhysicsShapeAutoCompound() {

    }

    /**
     * @return
     * @hide
     */
    @Override
    public String getType() {
        return "compound";
    }

    /**
     * @return
     * @hide
     */
    @Override
    public float[] getParams() {
        return new float[0];
    }
}