package com.viro.renderer.jni;

/**
 * Represents a 3-component floating-point vector.
 *
 * TODO Add equals and hashCode
 */
public class Vector {

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
     * Construct a new Vector centered at the origin.
     */
    public Vector() {

    }

    /**
     * Construct a new Vector from the given coordinates.
     *
     * @param x The X coordinate.
     * @param y The Y coordinate.
     * @param z The Z coordinate.
     */
    public Vector(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    /**
     * Construct a new Vector from the given double coordinates by
     * downcasting them to floats.
     *
     * @param x The X coordinate.
     * @param y The Y coordinate.
     * @param z The Z coordinate.
     */
    public Vector(double x, double y, double z) {
        this((float) x, (float) y, (float) z);
    }

    /**
     * Construct a new Vector from the given coordinates in an array of length 3.
     *
     * @param coordinates The x, y, and z coordinates.
     */
    public Vector(float[] coordinates) {
        this.x = coordinates[0];
        this.y = coordinates[1];
        if (coordinates.length > 2) {
            this.z = coordinates[2];
        }
    }

    /**
     * Get the contents of the Vector in an array.
     *
     * @return Float array of length three with components [x, y, z].
     */
    public float[] toArray() {
        return new float[]{x, y, z};
    }

    @Override
    public String toString() {
        return "[" + x + ", " + y + ", " + z + "]";
    }
}
