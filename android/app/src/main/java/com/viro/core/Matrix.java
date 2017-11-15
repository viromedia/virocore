/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

import java.util.Arrays;

/**
 * Matrix represents a 4x4 floating-point Matrix. Values are column-major.
 */
public class Matrix {

    /**
     * Return a new identity matrix.
     *
     * @return Identity matrix.
     */
    public static Matrix makeIdentity() {
        return new Matrix();
    }

    /**
     * Return a new scale matrix. Multiplying this Matrix with a {@link Vector} will scale the
     * Vector by the given amounts.
     *
     * @param sx The X scale.
     * @param sy The Y scale.
     * @param sz The Z scale.
     * @return The scale Matrix.
     */
    public static Matrix makeScale(float sx, float sy, float sz) {
        Matrix matrix = new Matrix();
        matrix.values[0] = sx;
        matrix.values[5] = sy;
        matrix.values[10] = sz;
        return matrix;
    }

    /**
     * Return a new translation matrix. Multiplying this Matrix with a {@link Vector} will translate
     * the Vector by the given amounts.
     *
     * @param tx The X translation.
     * @param ty The Y translation.
     * @param tz The Z translation.
     * @return The translation Matrix.
     */
    public static Matrix makeTranslation(float tx, float ty, float tz) {
        Matrix matrix = new Matrix();
        matrix.values[12] = tx;
        matrix.values[13] = ty;
        matrix.values[14] = tz;
        return matrix;
    }

    /**
     * Return a new rotation matrix about the given axis. Multiplying this Matrix with a {@link
     * Vector} will rotate the vector by the given radians about the provided axis.
     *
     * @param angleRad The rotation amount in radians.
     * @param origin   The origin of the axis of rotation.
     * @param dir      The direction of the axis of rotation.
     * @return The rotation Matrix.
     */
    public static Matrix makeRotation(float angleRad, Vector origin, Vector dir) {
        float a = origin.x;
        float b = origin.y;
        float c = origin.z;

        float u = dir.x;
        float v = dir.y;
        float w = dir.z;
        float u2 = u * u;
        float v2 = v * v;
        float w2 = w * w;
        float l2 = u2 + v2 + w2;

        // Early out for short rotation vector
        if (l2 < 0.000000001f) {
            return makeIdentity();
        }

        float normalizedAngle = normalizeAnglePI(angleRad);
        float sinT = (float) Math.sin(normalizedAngle);
        float cosT = (float) Math.cos(normalizedAngle);
        float l = (float) Math.sqrt(l2);

        Matrix txMtx = new Matrix();
        txMtx.values[0] = (u2 + (v2 + w2) * cosT) / l2;
        txMtx.values[1] = (u * v * (1 - cosT) + w * l * sinT) / l2;
        txMtx.values[2] = (u * w * (1 - cosT) - v * l * sinT) / l2;
        txMtx.values[3] = 0;

        txMtx.values[4] = (u * v * (1 - cosT) - w * l * sinT) / l2;
        txMtx.values[5] = (v2 + (u2 + w2) * cosT) / l2;
        txMtx.values[6] = (v * w * (1 - cosT) + u * l * sinT) / l2;
        txMtx.values[7] = 0;

        txMtx.values[8] = (u * w * (1 - cosT) + v * l * sinT) / l2;
        txMtx.values[9] = (v * w * (1 - cosT) - u * l * sinT) / l2;
        txMtx.values[10] = (w2 + (u2 + v2) * cosT) / l2;
        txMtx.values[11] = 0;

        txMtx.values[12] = ((a * (v2 + w2) - u * (b * v + c * w)) * (1 - cosT) + (b * w - c * v) * l * sinT) / l2;
        txMtx.values[13] = ((b * (u2 + w2) - v * (a * u + c * w)) * (1 - cosT) + (c * u - a * w) * l * sinT) / l2;
        txMtx.values[14] = ((c * (u2 + v2) - w * (a * u + b * v)) * (1 - cosT) + (a * v - b * u) * l * sinT) / l2;
        txMtx.values[15] = 1;

        return txMtx;
    }


    /**
     * The components of this matrix in column-major order. E.g. values[0] is the top-left
     * component, values[1] is first row, first column, values[1] is second row, first column,
     * values[4] is first row, second column, and so on.
     */
    float[] values;

    /**
     * Construct a new Matrix, initialized to identity.
     */
    public Matrix() {
        values = new float[16];
        makeIdentity(values);
    }

    /**
     * Construct a new Matrix with the given values in a float[16], in column-major order.
     *
     * @param values The column-major component values for the Matrix, in an array of length 16.
     */
    public Matrix(float[] values) {
        this.values = new float[16];
        System.arraycopy(values, 0, this.values, 0, 16);
    }

    /**
     * Multiply this Matrix by the given {@link Vector} and return the result (neither this Matrix
     * nor the input Vector are mutated by this operation).
     *
     * @param vector The {@link Vector} to multiply by this Matrix.
     * @return The result of the multiplication.
     */
    public Vector multiply(Vector vector) {
        Vector result = new Vector();
        result.x = vector.x * values[0] + vector.y * values[4] + vector.z * values[8] + values[12];
        result.y = vector.x * values[1] + vector.y * values[5] + vector.z * values[9] + values[13];
        result.z = vector.x * values[2] + vector.y * values[6] + vector.z * values[10] + values[14];
        return result;
    }

    /**
     * Multiply this Matrix by the given matrix and return the result (neither this Matrix nor the
     * input Matrix are mutated by this operation).
     *
     * @param matrix The Matrix to multiply by this one.
     * @return The product of the two matrices.
     */
    public Matrix multiply(Matrix matrix) {
        float[] nmtx = new float[16];
        multiply4x4(matrix.values, values, nmtx);
        return new Matrix(nmtx);
    }

    /**
     * Transpose this Matrix and return the result (this Matrix is not mutated by the operation).
     *
     * @return The transposed Matrix.
     */
    public Matrix transpose() {
        float[] transpose = new float[16];
        transpose4x4(values, transpose);
        return new Matrix(transpose);
    }

    /**
     * Invert this Matrix and return the result (this Matrix is not mutated by the operation).
     *
     * @return The inverted Matrix.
     */
    public Matrix invert() {
        float[] inverted = new float[16];
        invert4x4(values, inverted);
        return new Matrix(inverted);
    }

    /**
     * Extract the scale component of this Matrix and return it as a {@link Vector}. This Matrix
     * is not mutated by this operation.
     *
     * @return The scale (X, Y, and Z) embedded bin this Matrix.
     */
    public Vector extractScale() {
        Vector s0 = new Vector(values[0], values[1], values[2]);
        Vector s1 = new Vector(values[4], values[5], values[6]);
        Vector s2 = new Vector(values[8], values[9], values[10]);

        return new Vector(s0.magnitude(), s1.magnitude(), s2.magnitude());
    }

    /**
     * Extract the rotation embedded in this Matrix and return it as a {@link Quaternion}. This
     * matrix is not mutated by this operation. The scale component of this Matrix is required to
     * compute the rotation: this can be retrieved via {@link #extractScale()}.
     *
     * @param scale The scale component of the Matrix, as returned by {@link #extractScale()}.
     * @return The rotation component of this Matrix as a {@link Quaternion}.
     */
    public Quaternion extractRotation(Vector scale) {
        float[] mtx = {values[0] / scale.x,
                values[1] / scale.x,
                values[2] / scale.x,
                0,
                values[4] / scale.y,
                values[5] / scale.y,
                values[6] / scale.y,
                0,
                values[8] / scale.z,
                values[9] / scale.z,
                values[10] / scale.z,
                0,
                0, 0, 0, 1 };


        Quaternion result = new Quaternion();
        if (mtx[0] + mtx[5] + mtx[10] > 0.0f) {
            float t = + mtx[0] + mtx[5] + mtx[10] + 1.0f;
            float s = 1.f / (float) Math.sqrt(t) * 0.5f;

            result.w = s * t;
            result.z = ( mtx[1] - mtx[4] ) * s;
            result.y = ( mtx[8] - mtx[2] ) * s;
            result.x = ( mtx[6] - mtx[9] ) * s;
        }
        else if (mtx[0] > mtx[5] && mtx[0] > mtx[10]) {
            float t = + mtx[0] - mtx[5] - mtx[10] + 1.0f;
            float s = 1.0f / (float) Math.sqrt( t ) * 0.5f;

            result.x = s * t;
            result.y = ( mtx[1] + mtx[4] ) * s;
            result.z = ( mtx[8] + mtx[2] ) * s;
            result.w = ( mtx[6] - mtx[9] ) * s;
        }
        else if (mtx[5] > mtx[10]) {
            float t = - mtx[0] + mtx[5] - mtx[10] + 1.0f;
            float s = 1.0f / (float) Math.sqrt( t ) * 0.5f;
            result.y = s * t;
            result.x = ( mtx[1] + mtx[4] ) * s;
            result.w = ( mtx[8] - mtx[2] ) * s;
            result.z = ( mtx[6] + mtx[9] ) * s;
        }
        else {
            float t = - mtx[0] - mtx[5] + mtx[10] + 1.0f;
            float s = 1.0f / (float) Math.sqrt( t ) * 0.5f;

            result.z = s * t;
            result.w = ( mtx[1] - mtx[4] ) * s;
            result.x = ( mtx[8] + mtx[2] ) * s;
            result.y = ( mtx[6] + mtx[9] ) * s;
        }
        return result.normalize();
    }

    /**
     * Extract the translation component of this Matrix and return it as a {@link Vector}. This
     * Matrix is not mutated by this operation.
     *
     * @return The translation (X, Y, and Z) embedded in this Matrix.
     */
    public Vector extractTranslation() {
        return new Vector(values[12], values[13], values[14]);
    }

    @Override
    public boolean equals(Object obj) {
        if (!(obj instanceof Matrix)) {
            return false;
        }
        if (obj == null) {
            return false;
        }
        Matrix matrix = (Matrix) obj;
        return Arrays.equals(values, matrix.values);
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(values);
    }

    @Override
    public String toString() {
        return "\n" +
        values[0] + ", " + values[4] + ", " + values[8] + ", " + values[12] + "\n" +
        values[1] + ", " + values[5] + ", " + values[9] + ", " + values[13] + "\n" +
        values[2] + ", " + values[6] + ", " + values[10] + ", " + values[14] + "\n" +
        values[3] + ", " + values[7] + ", " + values[11] + ", " + values[15];
    }

    private static void multiply4x4(float[] m1, float[] m0, float[] d) {
        d[0] = m0[0] * m1[0] + m0[4] * m1[1] + m0[8] * m1[2] + m0[12] * m1[3];
        d[1] = m0[1] * m1[0] + m0[5] * m1[1] + m0[9] * m1[2] + m0[13] * m1[3];
        d[2] = m0[2] * m1[0] + m0[6] * m1[1] + m0[10] * m1[2] + m0[14] * m1[3];
        d[3] = m0[3] * m1[0] + m0[7] * m1[1] + m0[11] * m1[2] + m0[15] * m1[3];
        d[4] = m0[0] * m1[4] + m0[4] * m1[5] + m0[8] * m1[6] + m0[12] * m1[7];
        d[5] = m0[1] * m1[4] + m0[5] * m1[5] + m0[9] * m1[6] + m0[13] * m1[7];
        d[6] = m0[2] * m1[4] + m0[6] * m1[5] + m0[10] * m1[6] + m0[14] * m1[7];
        d[7] = m0[3] * m1[4] + m0[7] * m1[5] + m0[11] * m1[6] + m0[15] * m1[7];
        d[8] = m0[0] * m1[8] + m0[4] * m1[9] + m0[8] * m1[10] + m0[12] * m1[11];
        d[9] = m0[1] * m1[8] + m0[5] * m1[9] + m0[9] * m1[10] + m0[13] * m1[11];
        d[10] = m0[2] * m1[8] + m0[6] * m1[9] + m0[10] * m1[10] + m0[14] * m1[11];
        d[11] = m0[3] * m1[8] + m0[7] * m1[9] + m0[11] * m1[10] + m0[15] * m1[11];
        d[12] = m0[0] * m1[12] + m0[4] * m1[13] + m0[8] * m1[14] + m0[12] * m1[15];
        d[13] = m0[1] * m1[12] + m0[5] * m1[13] + m0[9] * m1[14] + m0[13] * m1[15];
        d[14] = m0[2] * m1[12] + m0[6] * m1[13] + m0[10] * m1[14] + m0[14] * m1[15];
        d[15] = m0[3] * m1[12] + m0[7] * m1[13] + m0[11] * m1[14] + m0[15] * m1[15];
    }

    private static void transpose4x4(float[] src, float[] transpose) {
        transpose[0] = src[0];
        transpose[1] = src[4];
        transpose[2] = src[8];
        transpose[3] = src[12];

        transpose[4] = src[1];
        transpose[5] = src[5];
        transpose[6] = src[9];
        transpose[7] = src[13];

        transpose[8] = src[2];
        transpose[9] = src[6];
        transpose[10] = src[10];
        transpose[11] = src[14];

        transpose[12] = src[3];
        transpose[13] = src[7];
        transpose[14] = src[11];
        transpose[15] = src[15];
    }

    private static void invert4x4(float[] src, float[] inverse) {
        float[] temp = new float[16];
        int i, j, k, swap;
        float t;

        for (int m = 0; m < 16; m++) {
            temp[m] = src[m];
        }

        makeIdentity(inverse);

        for (i = 0; i < 4; i++) {
        /*
         * Look for largest element in column
         */
            swap = i;
            for (j = i + 1; j < 4; j++) {
                if (Math.abs((int)temp[(j << 2) + i]) > Math.abs((int)temp[(i << 2) + i])) {
                    swap = j;
                }
            }

            if (swap != i) {
            /*
             * Swap rows.
             */
                for (k = 0; k < 4; k++) {
                    t = temp[(i << 2) + k];
                    temp[(i << 2) + k] = temp[(swap << 2) + k];
                    temp[(swap << 2) + k] = t;

                    t = inverse[(i << 2) + k];
                    inverse[(i << 2) + k] = inverse[(swap << 2) + k];

                    inverse[(swap << 2) + k] = t;
                }
            }

            if (temp[(i << 2) + i] == 0) {
            /*
             No non-zero pivot. The matrix is singular, which shouldn't
             happen. This means the user gave us a bad matrix.
             */
                return;
            }

            t = temp[(i << 2) + i];
            for (k = 0; k < 4; k++) {
                temp[(i << 2) + k] /= t;
                inverse[(i << 2) + k] = inverse[(i << 2) + k] / t;
            }
            for (j = 0; j < 4; j++) {
                if (j != i) {
                    t = temp[(j << 2) + i];
                    for (k = 0; k < 4; k++) {
                        temp[(j << 2) + k] -= temp[(i << 2) + k] * t;
                        inverse[(j << 2) + k] = inverse[(j << 2) + k] - inverse[(i << 2) + k] * t;
                    }
                }
            }
        }
    }

    private static void makeIdentity(float[] values) {
        for (int i = 0; i < 16; i++) {
            values[i] = 0;
        }
        values[0] = values[5] = values[10] = values[15] = 1;
    }

    private static float normalizeAnglePI(float rad) {
        if (rad > -Math.PI && rad < Math.PI) {
            return rad;
        }

        float twopi = 2 * (float)Math.PI;
        float numCirclesStart = (float) Math.floor(rad / twopi);
        rad = rad - numCirclesStart * twopi;

        if (rad < -Math.PI) {
            return twopi - rad;
        }
        else if (rad > Math.PI) {
            return rad - twopi;
        }
        else {
            return rad;
        }
    }

}
