package com.viro.renderer.jni;

/**
 * Represents a 4-component floating-point quaternion.
 *
 * TODO Add all methods
 */
public class Quaternion {

    /**
     * The X coordinate.
     */
    public float x;

    /**
     * The Y coordinate.
     */
    public float y;

    /**
     * The Z coordinate.
     */
    public float z;

    /**
     * The W coordinate.
     */
    public float w;

    /**
     * Construct a new Quaternion.
     */
    public Quaternion() {

    }

    /**
     * Construct a new Quaternion from the given coordinates.
     *
     * @param x The X coordinate.
     * @param y The Y coordinate.
     * @param z The Z coordinate.
     * @param w The W coordinate.
     */
    public Quaternion(float x, float y, float z, float w) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }

    /**
     * Construct a new Quaternion from the given coordinates in an array of length r.
     *
     * @param coordinates The x, y, z, and w coordinates.
     */
    public Quaternion(float[] coordinates) {
        this.x = coordinates[0];
        this.y = coordinates[1];
        this.z = coordinates[2];
        this.w = coordinates[3];
    }

    /**
     * Get the contents of the Quaternion in an array.
     *
     * @return Float array of length four with components [x, y, z, w].
     */
    public float[] toArray() {
        return new float[]{x, y, z, w};
    }

}
