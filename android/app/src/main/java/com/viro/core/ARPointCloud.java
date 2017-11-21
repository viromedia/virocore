/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

package com.viro.core;

/**
 * ARPointCloud contains a collection of points that the AR subsystem has detected in the
 * user's real world.
 */
public class ARPointCloud {
    private float[] mPoints;

    ARPointCloud(float[] points) {
        // The reason why ARPointCloud simply contains a float[] is because each frame can contain
        // hundreds of points which, if converted to Vectors would result in hundreds of Java objects
        // created each frame (I tried this and would overflow local memory space in JNI).
        mPoints = points;
    }

    /**
     * Returns a float array containing the (x,y,z) position of each point in the point cloud
     * plus a confidence value (from 0 to 1). Each point therefore takes up 4 floats in the array
     * and are arranged in no particular order.
     *
     * @return A single float array containing the position and confidence of each point.
     */
    public float[] getPoints() {
        return mPoints;
    }

    /**
     * Returns the number of points in the ARPointCloud.
     *
     * @return the number of points in the ARPointCloud.
     */
    public int size() {
        return mPoints.length / 4;
    }
}

