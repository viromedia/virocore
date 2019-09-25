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

import java.util.Arrays;

/**
 * Represents a 4-component floating-point quaternion.
 */
public class Quaternion {

    /**
     * Construct and return the identity Quaternion.
     *
     * @return The identity Quaternion.
     */
    public static Quaternion makeIdentity() {
       return new Quaternion(0, 0, 0, 1);
    }

    /**
     * Make a {@link Quaternion} that rotates about the given axis by the given angle in radians.
     *
     * @param angle The magnitude of the rotation in radians.
     * @param axis  The axis about which rotation will occur. Must be unit length.
     * @return The {@link Quaternion} representing rotation of <tt>angle</tt> radians about
     * <tt>axis</tt>.
     */
    public Quaternion makeRotation(float angle, Vector axis) {
        float fHalfAngle = 0.5f * angle;
        float fSin = (float) Math.sin(fHalfAngle);

        Quaternion q = new Quaternion();
        q.w = (float) Math.cos(fHalfAngle);
        q.x = fSin * axis.x;
        q.y = fSin * axis.y;
        q.z = fSin * axis.z;

        return q;
    }

    /**
     * Construct and return a Quaternion that rotates the ray <tt>from</tt> to the
     * ray <tt>to</tt>. In other words, when multiplied by this Quaternion, <tt>from</tt>
     * will become <tt>to</tt>.
     *
     * @param from The Quaternion will move this {@link Vector} to the <tt>to</tt> Vector.
     * @param to   The destination {@link Vector}.
     * @return The Quaternion to rotate <tt>from</tt> to <tt>to</tt>.
     */
    public static Quaternion makeRotationFromTo(Vector from, Vector to) {
        Vector v0 = from.normalize();
        Vector v1 = to.normalize();

        float d = v0.dot(v1);
        if (d >= 1.0f) {
            return makeIdentity();
        } else if (d <= -1.0f) {
            Vector axis = new Vector(1.0f, 0.f, 0.f);
            axis = axis.cross(v0);
            if (axis.magnitude() == 0) {
                axis.set(0.f, 1.f, 0.f);
                axis = axis.cross(v0);
            }

            Quaternion q = new Quaternion(axis.x, axis.y, axis.z, 0);
            return q.normalize();
        }

        float s = (float) Math.sqrt((1 + d) * 2);
        float invs = 1.f / s;
        Vector c = v0.cross(v1).scale(invs);

        Quaternion q = new Quaternion(c.x, c.y, c.z, s * 0.5f);
        return q.normalize();
    }

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
        this.x = 0;
        this.y = 0;
        this.z = 0;
        this.w = 1;
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
     * Construct a new Quaternion from the given coordinates in an array of length 4.
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
     * Construct a new Quaternion from the given Euler angles about each axis, in radians.
     *
     * @param x The X-axis rotation in radians.
     * @param y The Y-axis rotation in radians.
     * @param z The Z-axis rotation in radians.
     */
    public Quaternion(float x, float y, float z) {
        set(x, y, z);
    }

    /**
     * Construct a new Quaternion from the given Euler angles in radians. The angles are provided
     * as components in a {@link Vector}.
     *
     * @param vec The vector containing the Euler angle rotation.
     */
    public Quaternion(Vector vec) {
        set(vec.x, vec.y, vec.z);
    }

    /**
     * Construct a new Quaternion by copying the given Quaternion.
     *
     * @param quaternion The Quaternion to copy.
     */
    public Quaternion(Quaternion quaternion) {
        set(quaternion);
    }

    /**
     * Construct a new Quaternion from the given rotation {@link Matrix}.
     *
     * @param matrix The rotation Matrix to convert into a Quaternion.
     */
    public Quaternion(Matrix matrix) {
        set(matrix);
    }

    // Euler angle setter
    private void set(float x, float y, float z) {
        double angle;

        angle = x * 0.5;
        double sr = (float) Math.sin(angle);
        double cr = (float) Math.cos(angle);

        angle = y * 0.5;
        double sp = (float) Math.sin(angle);
        double cp = (float) Math.cos(angle);

        angle = z * 0.5;
        double sy = (float) Math.sin(angle);
        double cy = (float) Math.cos(angle);

        double cpcy = cp * cy;
        double spcy = sp * cy;
        double cpsy = cp * sy;
        double spsy = sp * sy;

        this.x = (float) (sr * cpcy - cr * spsy);
        this.y = (float) (cr * spcy + sr * cpsy);
        this.z = (float) (cr * cpsy - sr * spcy);
        this.w = (float) (cr * cpcy + sr * spsy);

        set(normalize());
    }

    private void set(Quaternion quat) {
        this.x = quat.x;
        this.y = quat.y;
        this.z = quat.z;
        this.w = quat.w;
    }

    private Quaternion set(Matrix m) {
        float diag = m.values[0] + m.values[5] + m.values[10] + 1;
        if (diag > 0.0f) {
            float scale = (float) Math.sqrt(diag) * 2.0f; // get scale from diagonal

            // TODO: speed this up
            x = (m.values[6] - m.values[9]) / scale;
            y = (m.values[8] - m.values[2]) / scale;
            z = (m.values[1] - m.values[4]) / scale;
            w = 0.25f * scale;
        } else {
            if (m.values[0] > m.values[5] && m.values[0] > m.values[10]) {
                // 1st element of diag is greatest value
                // find scale according to 1st element, and double it
                float scale = (float) Math.sqrt(1.0f + m.values[0] - m.values[5] - m.values[10]) * 2.0f;

                // TODO: speed this up
                x = 0.25f * scale;
                y = (m.values[4] + m.values[1]) / scale;
                z = (m.values[2] + m.values[8]) / scale;
                w = (m.values[6] - m.values[9]) / scale;
            } else if (m.values[5] > m.values[10]) {
                // 2nd element of diag is greatest value
                // find scale according to 2nd element, and double it
                float scale = (float) Math.sqrt(1.0f + m.values[5] - m.values[0] - m.values[10]) * 2.0f;

                x = (m.values[4] + m.values[1]) / scale;
                y = 0.25f * scale;
                z = (m.values[9] + m.values[6]) / scale;
                w = (m.values[8] - m.values[2]) / scale;
            } else {
                // 3rd element of diag is greatest value
                // find scale according to 3rd element, and double it
                float scale = (float) Math.sqrt(1.0f + m.values[10] - m.values[0] - m.values[5]) * 2.0f;

                x = (m.values[8] + m.values[2]) / scale;
                y = (m.values[9] + m.values[6]) / scale;
                z = 0.25f * scale;
                w = (m.values[1] - m.values[4]) / scale;
            }
        }
        return normalize();
    }

    /**
     * Multiply this Quaternion by the given Quaternion and return the result. Neither Quaternion
     * is mutated by this operation.
     *
     * @param other The Quaternion to multiply by.
     * @return The product of the two Quaternions.
     */
    public Quaternion multiply(Quaternion other) {
        Quaternion tmp = new Quaternion();
        tmp.w = (other.w * w) - (other.x * x) - (other.y * y) - (other.z * z);
        tmp.x = (other.w * x) + (other.x * w) + (other.y * z) - (other.z * y);
        tmp.y = (other.w * y) + (other.y * w) + (other.z * x) - (other.x * z);
        tmp.z = (other.w * z) + (other.z * w) + (other.x * y) - (other.y * x);

        return tmp;
    }

    /**
     * Multiply this Quaternion by the given scalar value and return the result. This Quaternion
     * is not mutated by this operation.
     *
     * @param s The scalar value to multiply by.
     * @return The scaled Quaternion.
     */
    public Quaternion multiply(float s) {
        return new Quaternion(s * x, s * y, s * z, s * w);
    }

    /**
     * Multiply the given {@link Vector} by this Quaternion and return the result. Neither the
     * Quaternion nor the Vector are mutated by this operation.
     *
     * @param v The {@link Vector} to be multiplied.
     * @return The result of the multiplication.
     */
    public Vector multiply(Vector v) {
        Vector qvec = new Vector(x, y, z);
        Vector uv = qvec.cross(v);
        Vector uuv = qvec.cross(uv);
        uv = uv.scale(2.0f * w);
        uuv = uuv.scale(2.0f);
        return v.add(uv).add(uuv);
    }

    /**
     * Add this Quaternion to the given Quaternion and return the result. Neither Quaternion
     * is mutated by this operation.
     *
     * @param b The Quaternion to add.
     * @return The sum of the two Quaternions.
     */
    Quaternion add(Quaternion b) {
        return new Quaternion(x + b.x, y + b.y, z + b.z, w + b.w);
    }

    /**
     * Subtract the given Quaternion from this Quaternion and return the result. Neither Quaternion
     * is mutated by this operation.
     *
     * @param b The Quaternion to subtract.
     * @return The difference after subtracting the given Quaternion from this Quaternion.
     */
    public Quaternion subtract(Quaternion b) {
        return new Quaternion(x - b.x, y - b.y, z - b.z, w - b.w);
    }

    /**
     * Convert this Quaternion into a rotation {@link Matrix}.
     *
     * @return The rotation Matrix generated from this Quaternion.
     */
    public Matrix getMatrix() {
        Matrix dest = new Matrix();
        dest.values[0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
        dest.values[1] = 2.0f * x * y + 2.0f * z * w;
        dest.values[2] = 2.0f * x * z - 2.0f * y * w;
        dest.values[3] = 0.0f;
        dest.values[4] = 2.0f * x * y - 2.0f * z * w;
        dest.values[5] = 1.0f - 2.0f * x * x - 2.0f * z * z;
        dest.values[6] = 2.0f * z * y + 2.0f * x * w;
        dest.values[7] = 0.0f;
        dest.values[8] = 2.0f * x * z + 2.0f * y * w;
        dest.values[9] = 2.0f * z * y - 2.0f * x * w;
        dest.values[10] = 1.0f - 2.0f * x * x - 2.0f * y * y;
        dest.values[11] = 0.0f;
        dest.values[12] = 0;//center.x;
        dest.values[13] = 0;//center.y;
        dest.values[14] = 0;//center.z;
        dest.values[15] = 1.f;
        return dest;
    }

    /**
     * Invert this Quaternion and return the result. This Quaternion is not mutated by this
     * operation.
     *
     * @return The inverted Quaternion.
     */
    public Quaternion invert() {
        return new Quaternion(-x, -y, -z, w);
    }

    /**
     * Compute the dot product of this Quaternion with the given Quaternion, and return the
     * result. Neither Quaternion is mutated by this operation.
     *
     * @param q2 The Quaternion with which to dot this Quaternion.
     * @return The dot product of the two Quaternions.
     */
    public float dot(Quaternion q2) {
        return (x * q2.x) + (y * q2.y) + (z * q2.z) + (w * q2.w);
    }

    /**
     * Normalize this Quaternion and return the result. This Quaternion is not mutated by this
     * operation.
     *
     * @return The normalized Quaternion.
     */
    public Quaternion normalize() {
        float n = x * x + y * y + z * z + w * w;
        if (n == 1) {
            return new Quaternion(this);
        }
        else {
            return multiply(1 / (float) Math.sqrt(n));
        }
    }

    /**
     * Get the norm of this Quaternion.
     *
     * @return The norm of the Quaternion.
     */
    public float getNorm() {
        return (float) Math.sqrt(dot(this));
    }

    /**
     * Linearly interpolate between this Quaternion and the given Quaternion, and return the result.
     * Neither Quaternion is mutated by this operation.
     *
     * @param q The Quaternion to interpolate toward.
     *  @param t The interpolation factor between 0 and 1, where 0 corresponds to this Quaternion, 1
     *             corresponds to <tt>q</tt>, and values in-between result in an interpolated Quaternion between the two.
     * @return The interpolated Quaternion.
     */
    public Quaternion lerp(Quaternion q, float t) {
        float scale = 1.0f - t;
        return (this.multiply(scale)).add(q.multiply(t));
    }

    // set this quaternion to the result of the interpolation between two quaternions

    /**
     * Perform a spherical linear interpolation between this Quaternion and the given Quaternion,
     * and return the result. Slerp results in constant-speed motion along a unit-radius great
     * circle arc between the two Quaternions.
     *
     * @param q         The Quaternion to interpolate toward.
     * @param t         The interpolation factor between 0 and 1, where 0 corresponds to this
     *                  Quaternion, 1 corresponds to <tt>q</tt>, and values in-between result in an
     *                  interpolated Quaternion between the two.
     * @param threshold To avoid inaccuracies near the end (t = 1) the interpolation switches to
     *                  linear interpolation at some point. This value defines how much of the
     *                  remaining interpolation will be calculated with lerp. Everything from
     *                  1-threshold up will be linear interpolation. A reasonable default threshold
     *                  is 0.05f.
     * @return The interpolated Quaternion.
     */
    public Quaternion slerp(Quaternion q, float t, float threshold) {
        float angle = this.dot(q);

        // make sure we use the short rotation
        Quaternion q0 = new Quaternion(this);
        if (angle < 0.0f) {
            q0 = q0.multiply(-1.0f);
            angle *= -1.0f;
        }

        if (angle <= (1 - threshold)) {
            float theta = (float) Math.acos(angle);
            float invsintheta = 1.0f / (float) Math.sin(theta);
            float scale = (float) Math.sin(theta * (1.0f - t)) * invsintheta;
            float invscale = (float) Math.sin(theta * t) * invsintheta;

            return (q0.multiply(scale)).add(q.multiply(invscale));
        }
        else { // linear interpolation
            return q0.lerp(q, t);
        }
    }

    /**
     * Get the axis of rotation and the angle of rotation (in radians) that this Quaternion
     * represents. Store the axis in the given output {@link Vector}.
     *
     * @param axis The axis computed will be stored here.
     * @return The angle of rotation in radians.
     */
    public float toAngleAxis(Vector axis) {
        float angle;
        float scale = (float) Math.sqrt(x * x + y * y + z * z);

        if (Math.abs(scale) < 0.000001f || w > 1.0f || w < -1.0f) {
            angle = 0.0f;
            axis.x = 0.0f;
            axis.y = 1.0f;
            axis.z = 0.0f;
        } else {
            float invscale = 1f / scale;
            angle = 2.0f * (float) Math.acos(w);
            axis.x = x * invscale;
            axis.y = y * invscale;
            axis.z = z * invscale;
        }
        return angle;
    }

    /**
     * Get the angle, in radians, of the rotation this Quaternion represents. Note this method does
     * not specify about which <i>axis</i> the rotation will occur. To retrieve the axis use {@link
     * #toAngleAxis(Vector)}.
     *
     * @return The angle of rotation in radians.
     */
    public float getAngle() {
        float scale = (float) Math.sqrt(x * x + y * y + z * z);
        if (Math.abs(scale) < 0.000001f || w > 1.0f || w < -1.0f) {
            return 0;
        } else {
            return 2.0f * (float) Math.acos(w);
        }
    }

    /**
     * Convert this Quaternion into Euler rotation about each principal axis, and return the
     * result in a {@link Vector}. This Quaternion is not mutated by this operation.
     *
     * @return The Euler rotation about each axis (X, Y, and Z) in radians.
     */
    public Vector toEuler() {
        Vector euler = new Vector();
        double sqw = w * w;
        double sqx = x * x;
        double sqy = y * y;
        double sqz = z * z;
        float test = 2.0f * (y * w - x * z);

        if (equals(test, 1.0f, 0.000001f)) {
            // heading = rotation about z-axis
            euler.z = (float) (-2.0 * Math.atan2(x, w));
            // bank = rotation about x-axis
            euler.x = 0;
            // attitude = rotation about y-axis
            euler.y = (float) (Math.PI / 2.0);
        } else if (equals(test, -1.0f, 0.000001f)) {
            // heading = rotation about z-axis
            euler.z = (float) (2.0 * Math.atan2(x, w));
            // bank = rotation about x-axis
            euler.x = 0;
            // attitude = rotation about y-axis
            euler.y = (float) (Math.PI / -2.0);
        } else {
            // heading = rotation about z-axis
            euler.z = (float) Math.atan2(2.0 * (x * y + z * w), (sqx - sqy - sqz + sqw));
            // bank = rotation about x-axis
            euler.x = (float) Math.atan2(2.0 * (y * z + x * w), (-sqx - sqy + sqz + sqw));
            // attitude = rotation about y-axis
            euler.y = (float) Math.asin(clamp(test, -1.0f, 1.0f));
        }
        return euler;
    }

    /**
     * Get the contents of the Quaternion in an array.
     *
     * @return Float array of length four with components [x, y, z, w].
     */
    public float[] toArray() {
        return new float[]{x, y, z, w};
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Quaternion)) {
            return false;
        }
        if (obj == null) {
            return false;
        }
        Quaternion q = (Quaternion) obj;
        float tolerance = 0.000001f;
        return Math.abs(x - q.x) < tolerance &&
                Math.abs(y - q.y) < tolerance &&
                Math.abs(z - q.z) < tolerance &&
                Math.abs(w - q.w) < tolerance;

    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(new float[] { x, y, z, w });
    }

    @Override
    public String toString() {
        return "[x: " + x  + ", y: " + y + ", z: " + z + ", w: " + w + "]";
    }

    private static boolean equals(float a, float b, float tolerance) {
        return (a + tolerance >= b) && (a - tolerance <= b);
    }

    private static float clamp(float val, float min, float max) {
        return Math.max(min, Math.min(max, val));
    }

}
