/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the  LICENSE file in the
 * root directory of this source tree. An additional grant  of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viro.renderer.jni;

/**
 * Callback interface for responding to AR hit-test results. These hit-tests are initiated
 * through the {@link ViroViewARCore}. Hit-tests are used to search the {@link ARScene} for
 * real-world objects.
 */
public interface ARHitTestCallback {

    /**
     * Invoked when an AR hit-test has completed. As each hit-test can intersect with multiple
     * real-world objects, an array of results is returned.
     *
     * @param results The results of the hit-test, each in an {@link ARHitTestResult}.
     */
    void onHitTestFinished(ARHitTestResult[] results);
}
