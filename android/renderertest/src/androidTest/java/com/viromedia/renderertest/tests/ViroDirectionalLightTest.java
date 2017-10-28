/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest.tests;

import android.support.test.runner.AndroidJUnit4;

import org.junit.Test;
import org.junit.runner.RunWith;

/**
 * Created by manish on 10/25/17.
 */

@RunWith(AndroidJUnit4.class)
public class ViroDirectionalLightTest extends ViroBaseTest{
    private static final String TAG = ViroDirectionalLightTest.class.getName();

    @Override
    void configureTestScene() {
    }

    @Test
    public void testDirectionalLight() {

        testColorChange();

        testIntensityChange();

        testShadoBiasChange();
    }

    private void testColorChange() {
        // test
        // change color to green
        assertPass("Changed color from yellow to green");
    }

    private void testIntensityChange() {

        assertPass("Changed intensity from 0.5 to 1.0");
    }

    private void testShadoBiasChange() {

        assertPass("Changed shadowBias from something to something");
    }
}
