package com.viro.renderer.jni;

/**
 * Spherical {@link PhysicsShape}.
 */
public class PhysicsShapeSphere implements PhysicsShape {
    private float mRadius;

    /**
     * Construct a new Sphere physics shape.
     * @param radius The radius of the sphere.
     */
    public PhysicsShapeSphere(float radius) {
        mRadius = radius;
    }

    /**
     * Get the radius of this Sphere.
     *
     * @return The radius.
     */
    public float getRadius() {
        return mRadius;
    }

    /**
     * @hide
     * @return
     */
    @Override
    public String getType() {
        return "sphere";
    }

    /**
     * @hide
     * @return
     */
    @Override
    public float[] getParams() {
        return new float[] { mRadius };
    }
}
