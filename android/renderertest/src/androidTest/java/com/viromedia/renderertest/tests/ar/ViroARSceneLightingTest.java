
/*
 * Copyright (c) 2017-present, ViroMedia, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the LICENSE file in the
 * root directory of this source tree. An additional grant of patent rights can be found in
 * the PATENTS file in the same directory.
 */

package com.viromedia.renderertest.tests.ar;

import android.content.Context;
import android.support.test.InstrumentationRegistry;
import android.support.test.rule.ActivityTestRule;
import android.support.test.runner.AndroidJUnit4;


import com.viro.renderer.jni.GLListener;
import com.viromedia.renderertest.ViroReleaseTestActivity;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.runner.RunWith;

import java.util.concurrent.Callable;

import static org.awaitility.Awaitility.await;
import static org.junit.Assert.assertEquals;

/**
 * Created by manish on 10/25/17.
 */

@RunWith(AndroidJUnit4.class)
public class ViroARSceneLightingTest implements GLListener {
    private Boolean mGLInitialized = false;

    @Rule
    public ActivityTestRule<ViroReleaseTestActivity> rule
            = new ActivityTestRule<>(ViroReleaseTestActivity.class, true, true);

    @Before
    public void setUp() {
        await().until(glInitialized());
    }
    @Test
    public void useAppContext() throws Exception {
        // Context of the app under test.
        Context appContext = InstrumentationRegistry.getTargetContext();

        assertEquals("com.viromedia.renderertest.arcore", appContext.getPackageName());
    }


    @Override
    public void onGlInitialized() {
        mGLInitialized = true;
    }

    private Callable<Boolean> glInitialized() {
        return new Callable<Boolean>() {
            @Override
            public Boolean call() throws Exception {
                return mGLInitialized;
            }
        };
    }
}
