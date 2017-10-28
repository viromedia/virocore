/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest.tests;

import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.support.test.runner.AndroidJUnit4;
import android.util.Log;

import org.junit.Test;
import org.junit.runner.RunWith;

import static org.junit.Assert.assertEquals;

/**
 * Created by manish on 10/25/17.
 */

@RunWith(AndroidJUnit4.class)
public class ViroAmbientLightTest extends ViroBaseTest{
    private static final String TAG = ViroAmbientLightTest.class.getName();

    @Override
    void configureTestScene() {
    }

    @Test
    public void dummy_test() {
        // Context of the app under test.
        Log.d(TAG, "useApp called");
        Context appContext = InstrumentationRegistry.getTargetContext();

        assertEquals("com.viromedia.renderertest.gvr", appContext.getPackageName());
        assertEquals(true, mActivity.isGlInitialized());
    }
}
