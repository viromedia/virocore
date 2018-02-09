/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */

package com.viro.core;

/**
 * Interface for receiving point cloud updates.
 */
public interface PointCloudUpdateListener {
    /**
     * Invoked when the renderer has new point cloud data available.
     *
     * @param pointCloud a collection of points, see {@link ARPointCloud}.
     */
    public void onUpdate(ARPointCloud pointCloud);
}
