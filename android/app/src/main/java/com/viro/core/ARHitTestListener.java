/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 */
package com.viro.core;

/**
 * Callback interface for responding to AR hit-test results. These hit-tests are initiated
 * through the {@link ViroViewARCore}. Hit-tests are used to search the {@link ARScene} for
 * real-world objects.
 */
public interface ARHitTestListener {

    /**
     * Invoked when an AR hit-test has completed. As each hit-test can intersect with multiple
     * real-world objects, an array of results is returned.
     *
     * @param results The results of the hit-test, each in an {@link ARHitTestResult}.
     */
    void onHitTestFinished(ARHitTestResult[] results);
}
