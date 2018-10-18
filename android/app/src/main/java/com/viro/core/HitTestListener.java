package com.viro.core;

/*
 * Copyright (c) 2018-present, ViroMedia, Inc.
 * All rights reserved.
 */

public interface HitTestListener {
    /**
     * Invoked when an AR hit-test has completed. As each hit-test can intersect with multiple
     * real-world objects, an array of results is returned.
     * @param results The results of the hit-test, each in an {@link HitTestResult}.
     **/

    void onHitTestFinished(HitTestResult[] results);
}



