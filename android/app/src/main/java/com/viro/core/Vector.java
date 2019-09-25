//
//  Copyright (c) 2017-present, ViroMedia, Inc.
//  All rights reserved.
//
//  Permission is hereby granted, free of charge, to any person obtaining
//  a copy of this software and associated documentation files (the
//  "Software"), to deal in the Software without restriction, including
//  without limitation the rights to use, copy, modify, merge, publish,
//  distribute, sublicense, and/or sell copies of the Software, and to
//  permit persons to whom the Software is furnished to do so, subject to
//  the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
//  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
//  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package com.viro.core;

/**
 * Vector represents a 3-component floating-point vector.
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
     * Zero out this Vector.
     */
    public void clear() {
        x = 0;
        y = 0;
        z = 0;
    }

    /**
     * Set this Vector to equal the given vector. Each component (x, y, z) will be copied from
     * the given Vector.
     *
     * @param value The Vector whose values we will copy into this Vector.
     */
    public void set(Vector value) {
        x = value.x;
        y = value.y;
        z = value.z;
    }

    /**
     * Set the X, Y, and X components of this Vector.
     *
     * @param x The X component.
     * @param y The Y component.
     * @param z The Z component.
     */
    public void set(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    /**
     * Determine if the ray defined by the given origin, in the direction of this Vector, intersects
     * the plane defined by the given point and normal vector.
     *
     * @param point  Any point on the plane.
     * @param normal The normal of the plane.
     * @param origin The origin of the ray.
     * @param intPt  Store the point of intersection, if any, between the ray and plane here.
     * @return True if the ray intersected the plane.
     */
    public boolean rayIntersectPlane(Vector point, Vector normal, Vector origin, Vector intPt) {
        float denom = dot(normal);
        if (denom == 0) {
            return false;
        }

        float c = normal.dot(point);
        float t = (c - normal.dot(origin)) / denom;
        if (t < 0) {
            return false;
        }

        intPt.x = origin.x + x * t;
        intPt.y = origin.y + y * t;
        intPt.z = origin.z + z * t;

        return true;
    }

    /**
     * Rotate this Vector about the given axis, and return the result (neither this Vector nor the
     * input Vector are mutated).
     *
     * @param axisDir  The direction of the axis we are rotating about.
     * @param axisPos  Any point on the axis we are rotating about.
     * @param angleRad The amount by which we should rotate, in radians.
     * @return The rotated Vector.
     */
    public Vector rotateAboutAxis(Vector axisDir, Vector axisPos, float angleRad) {
        return Matrix.makeRotation(angleRad, axisPos, axisDir).multiply(this);
    }

    /**
     * Return the dot product of this Vector with the given Vector.
     *
     * @param vB The Vector to dot with.
     * @return The result of the dot product.
     */
    public float dot(Vector vB) {
        return x * vB.x + y * vB.y + z * vB.z;
    }

    /**
     * Return the Euclidean distance between the position defined by this Vector and the position defined
     * by the given Vector.
     *
     * @param vB Return the distance to this position.
     * @return The distance between the two positions.
     */
    public float distance(Vector vB) {
        float dx = (vB.x - this.x);
        float dy = (vB.y - this.y);
        float dz = (vB.z - this.z);

        return (float) Math.sqrt(dx * dx + dy * dy + dz * dz);
    }

    /**
     * Add this Vector to the given Vector and return the result (neither this Vector nor the
     * input Vector are mutated).
     *
     * @param vB The Vector to add to this one.
     * @return The result.
     */
    public Vector add(Vector vB) {
        Vector result = new Vector();
        result.x = x + vB.x;
        result.y = y + vB.y;
        result.z = z + vB.z;

        return result;
    }

    /**
     * Subtract the given Vector from this Vector and return the result (neither this Vector nor the
     * input Vector are mutated).
     *
     * @param vB The Vector to subtract from this one.
     * @return The result.
     */
    public Vector subtract(Vector vB) {
        Vector result = new Vector();
        result.x = x - vB.x;
        result.y = y - vB.y;
        result.z = z - vB.z;

        return result;
    }

    /**
     * Return the midpoint between this Vector and the given Vector.
     *
     * @param other The Vector we return the midpoint from.
     * @return The midpoint between the two Vectors.
     */
    public Vector midpoint(Vector other) {
        Vector result = new Vector();
        result.x = (x + other.x) * 0.5f;
        result.y = (y + other.y) * 0.5f;
        result.z = (z + other.z) * 0.5f;

        return result;
    }

    /**
     * Compute the cross product between this Vector and the given Vector and return the result
     * (neither this Vector nor the input Vector are mutated).
     *
     * @param vB The Vector to cross with this one.
     * @return The cross product of the two Vectors.
     */
    public Vector cross(Vector vB) {
        Vector result = new Vector();
        result.x = y * vB.z - z * vB.y;
        result.y = z * vB.x - x * vB.z;
        result.z = x * vB.y - y * vB.x;

        return result;
    }

    /**
     * Normalize this vector and return the result (this Vector is not mutated).
     *
     * @return The normalized Vector.
     */
    public Vector normalize() {
        float inverseMag = 1.0f / (float) Math.sqrt(x * x + y * y + z * z);

        Vector result = new Vector();
        result.x = x * inverseMag;
        result.y = y * inverseMag;
        result.z = z * inverseMag;

        return result;
    }

    /**
     * Return the magnitude of this Vector.
     *
     * @return The magnitude.
     */
    public float magnitude() {
        return (float) Math.sqrt(x * x + y * y + z * z);
    }

    /**
     * Scale each component of this Vector by the given amount and return the result (this Vector is
     * not mutated).
     *
     * @param factor The factor by which to multiply each component of this Vector.
     * @return The scaled Vector.
     */
    public Vector scale(float factor) {
        Vector result = new Vector();
        result.x = x * factor;
        result.y = y * factor;
        result.z = z * factor;

        return result;
    }

    /**
     * Interpolate between this Vector and the given Vector by the given value (between 0 and 1),
     * and return the result (neither this Vector nor the input Vector are mutated).
     *
     * @param other The Vector to interpolate between.
     * @param t     The interpolation amount (where 0 is this Vector and 1 is the input Vector).
     * @return The interpolated Vector.
     */
    public Vector interpolate(Vector other, float t) {
        Vector result = new Vector();
        result.x = x + (other.x - x) * t;
        result.y = y + (other.y - y) * t;
        result.z = z + (other.z - z) * t;

        return result;
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
    public int hashCode() {
        return (int) Math.floor(x + 31 * y + 31 * z);
    }

    @Override
    public boolean equals(Object object) {
        if (!(object instanceof Vector)) {
            return false;
        }
        if (object == null) {
            return false;
        }
        Vector v = (Vector) object;
        return Math.abs(x - v.x) < .00001 && Math.abs(y - v.y) < .00001 && Math.abs(z - v.z) < .00001;
    }

    @Override
    public String toString() {
        return "[" + x + ", " + y + ", " + z + "]";
    }
}
