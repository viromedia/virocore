/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Callback interface for responding to hit-test results. These hit-tests are initiated
 * through the {@link ViroView}. Hit-tests are used to search the {@link Scene} for
 * virtual-world objects.
 */
public interface HitTestListener {

    /**
     * Invoked when a hit-test has completed. As each hit-test can intersect with multiple
     * world-world objects, an array of results is returned.
     *
     * @param results The results of the hit-test, each in an {@link HitTestResult}.
     */
    void onHitTestFinished(HitTestResult[] results);
    
}



